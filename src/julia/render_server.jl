module RenderServer

include("./scene_driver.jl")
using .SceneDriver

include("./server_security.jl")
using .ServerSecurity

import Base.Filesystem
import HTTP
import MbedTLS
import ZipFile
using Sockets

export StartSceneServer

struct SceneWithReferenceProvider
  scene_function::Function
  reference_provider::SceneDriver.ReferenceProvider
  context::Ptr{SceneDriver.Entify.Lib.EntifyContext}
end

function SceneWithReferenceProvider(
    width, height, create_scene_function::Function,
    context::Ptr{SceneDriver.Entify.Lib.EntifyContext})::SceneWithReferenceProvider
  reference_provider = SceneDriver.ReferenceProvider(context)
  try
    scene_function = create_scene_function(width, height, reference_provider)
    return SceneWithReferenceProvider(
        scene_function, reference_provider, context)
  catch e
    SceneDriver.References.ReleaseAllReferences(
              reference_provider, context)
    rethrow(e)
  end
end

function Release(scene::SceneWithReferenceProvider)
    SceneDriver.References.ReleaseAllReferences(
              scene.reference_provider, scene.context)
end

function StartSceneServer(
    password::String; bind_address::IPAddr=Sockets.localhost,
    port::UInt16=UInt16(4851))
  security_context = SecurityContext(password)

  kDefaultScenePath = joinpath(dirname(@__FILE__), "examples", "show_ip")

  current_scene = nothing

  scene_channel = Channel{Function}(1)
  server = Sockets.listen(bind_address, port)

  window_width = 1920
  window_height = 1080
  reference_provider = undef

  function HandleMessage(message::Vector{UInt8})::HTTP.Response
    @info "Received scene submission." message_length=length(message)

    zip_reader = ZipFile.Reader(IOBuffer(message))
    scene_dir = Filesystem.mktempdir()
    for f in zip_reader.files
      abs_filename = Filesystem.joinpath(scene_dir, f.name)
      Filesystem.mkpath(Filesystem.dirname(abs_filename))
      open(abs_filename, "w") do out_file
        write(out_file, read(f))
      end
    end
    close(zip_reader)

    try
      @info "New scene unpacked, loading..."
      create_scene_function = SceneDriver.LoadCreateSceneFunction(
          scene_dir)
      @info "Scene loaded, activating..."
      put!(scene_channel, create_scene_function)

    catch e
      bt = catch_backtrace()
      msg = sprint(showerror, e, bt)
      @error("Error activating new scene:\n$(msg)\n")

      return HTTP.Response(400, msg)
    end

    return HTTP.Response("Success!")
  end

  http_router = HTTP.Router()
  HTTP.@register(
      http_router, "POST", "/message",
      req -> HandleEncryptedMessage(security_context, HandleMessage, req))
  HTTP.@register(http_router, "GET", "/nonce", HandleGetNonce)

  SceneDriver.WithDefaultWindow() do window
    SceneDriver.WithContext() do context
      window_width = SceneDriver.Entify.Lib.PlatformWindowGetWidth(window)
      window_height = SceneDriver.Entify.Lib.PlatformWindowGetHeight(window)

  
      # If we're binding to any address, then it's not that useful to display
      # "0.0.0.0" on the display.
      ip_address_to_show = bind_address
      if bind_address == ip"0.0.0.0"
        ip_address_to_show = getipaddr()
      end

      reference_provider = SceneDriver.ReferenceProvider(context)
      default_scene_function =
          LoadCreateSceneFunction(kDefaultScenePath)(
              window_width, window_height, reference_provider,
              ip_address_to_show, port)
      default_scene = SceneWithReferenceProvider(
          default_scene_function, reference_provider, context)

      function GetCurrentScene(time_elapsed_in_seconds::Float32)
        yield()
        while isready(scene_channel)
          @info "Renderer switching to new scene."
          create_scene_function = take!(scene_channel)

          new_scene = SceneWithReferenceProvider(
              window_width, window_height, create_scene_function, context)

          # Release backend references from the previous scene before moving on.
          if current_scene != nothing
            Release(current_scene)
          end

          current_scene = new_scene
        end

        if current_scene == nothing
          return default_scene.scene_function(time_elapsed_in_seconds)
        end

        try
          return current_scene.scene_function(time_elapsed_in_seconds)
        catch e
          bt = catch_backtrace()
          msg = sprint(showerror, e, bt)
          @error(msg)

          if current_scene != nothing
            Release(current_scene)
          end
          current_scene = nothing
          return default_scene.scene_function(time_elapsed_in_seconds)
        end
      end

      try
        @async HTTP.serve(http_router; server=server)
        while true
          try
            SceneDriver.RenderSceneInWindow(window, context, GetCurrentScene)
            break
          catch e
            if e isa SceneDriver.Entify.ParseError
              if current_scene != nothing
                Release(current_scene)
              end
              current_scene = nothing
              bt = catch_backtrace()
              msg = sprint(showerror, e, bt)
              @error(msg)
            else
              rethrow(e)
            end
          end
        end
      finally
        close(server)
        Release(default_scene)
      end
    end
  end
end

end
