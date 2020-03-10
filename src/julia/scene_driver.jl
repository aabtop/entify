module SceneDriver

using Base.Filesystem
using Images
using StaticArrays

include("./entify.jl")
using .Entify

include("./references.jl")
using .References

export RenderSceneInWindow, LoadCreateSceneFunction, WithDefaultWindow,
       WithContext


function LoadCreateSceneFunction(scene_dir::String)::Function
  scene_filename = "main.jl"
  abs_scene_dir = abspath(scene_dir)
  scene_path = joinpath(abs_scene_dir, scene_filename)

  module_symbol = gensym("Scene")
  scene::Module = eval(Expr(:toplevel, :(module $module_symbol end)))
  Base.eval(scene, quote
      using LinearAlgebra
      using ..Entify
      using ..Entify.Affine
      using ..Entify.Shaders
      using ..Entify.ShaderCommonFunctions
      using ..Entify.Composites
      using ..Entify.Composites.PathShapes
      using ..Entify.Composites.Lines
      using ..Entify.Composites.Blit
      using ..Entify.Composites.Convolution
      using ..Entify.Composites.Shapes
      using ..Entify.Composites.SimplexNoiseModule
      using ..Entify.Composites.SoftMaxModule
      using ..References

      cd($abs_scene_dir) do
        include($scene_path)
      end
    end)

  function MakeDrawTreeFunction(args...)
    cd(abs_scene_dir) do
      # We use "@eval" here to bypass the problem of |create_scene_function|
      # belonging to a newer world generation than this function may have been
      # called in.
      draw_tree_or_function =
        @eval $scene.MakeDrawTree($(args...))

      draw_tree_function = undef
      if typeof(draw_tree_or_function) <: Entify.DrawTree
        # The client function is free to directly return a draw tree, in which
        # case we just treat it as a static image.
        draw_tree_function =
            function (time_in_seconds::Float32) draw_tree_or_function end
      else
        draw_tree_function = draw_tree_or_function
      end

      return function(args...)
        cd(abs_scene_dir) do
          @eval $draw_tree_function($(args...))
        end
      end
    end
  end

  return MakeDrawTreeFunction
end

function WithDefaultWindow(f::Function)
  window = Entify.Lib.PlatformWindowMakeDefaultWindow("Entify", C_NULL, C_NULL)
  if window == C_NULL
    error("There was a problem creating the default window.")
  end

  try
    f(window)
  finally
    Entify.Lib.PlatformWindowDestroyWindow(window)
  end   
end

function WithContext(f::Function)
  context = Entify.Lib.EntifyCreateContext()
  try
    f(context)
  finally
    Entify.Lib.EntifyDestroyContext(context)
  end
end

function RenderSceneInWindow(create_scene_function::Function)
  WithDefaultWindow() do window
    WithContext() do context
      window_width = Entify.Lib.PlatformWindowGetWidth(window)
      window_height = Entify.Lib.PlatformWindowGetHeight(window)

      reference_provider = ReferenceProvider(context)

      scene_function = create_scene_function(
          window_width, window_height, reference_provider)

      RenderSceneInWindow(window, context, scene_function)

      ReleaseAllReferences(reference_provider, context)
    end
  end
end

function RenderSceneInWindow(
    window::Ptr{Entify.Lib.PlatformWindow},
    context::Ptr{Entify.Lib.EntifyContext},
    scene_function::Function)
  window_width = Entify.Lib.PlatformWindowGetWidth(window)
  window_height = Entify.Lib.PlatformWindowGetHeight(window)

  render_target = Entify.Lib.EntifyCreateRenderTargetFromPlatformWindow(
      context, Entify.Lib.PlatformWindowGetNativeWindow(window),
      window_width, window_height)

  start_time = time_ns()
  GetElapsedTimeInSeconds() = (time_ns() - start_time) / 1000000000.0

  try
    frame_interval_times::Vector{Float32} = []
    submit_times::Vector{Float32} = []
    prev_time_elapsed = GetElapsedTimeInSeconds()
    while true
      time_elapsed_in_seconds::Float32 = GetElapsedTimeInSeconds()

      scene = scene_function(time_elapsed_in_seconds)
      submit_time = @elapsed(Submit(context, render_target, scene))

      push!(submit_times, Float32(submit_time))
      if length(submit_times) > 300
        average_submit_time_ms =
            1000.0 * reduce(+, submit_times) / length(submit_times)
        println("Average submit time: $(average_submit_time_ms)ms")
        submit_times = Vector{Float32}()
      end

      push!(frame_interval_times, time_elapsed_in_seconds - prev_time_elapsed)
      if length(frame_interval_times) > 300
        average_frame_time_ms =
            1000.0 * reduce(+, frame_interval_times) /
                length(frame_interval_times)

        println("Average frame time: $(average_frame_time_ms)ms")
        frame_interval_times = Vector{Float32}()
      end
      prev_time_elapsed = time_elapsed_in_seconds
    end
  catch e
    if e isa InterruptException
      println("Terminated by user interrupt.")
    else
      rethrow(e)
    end
  finally
    Entify.Lib.EntifyReleaseRenderTarget(context, render_target)
  end
end

function RenderSceneInWindow(scene_dir::String)
  create_scene_function = LoadCreateSceneFunction(scene_dir)
  RenderSceneInWindow(create_scene_function)
end

function RenderSceneInWindow(
    window::Ptr{Entify.Lib.PlatformWindow}, scene_dir::String)
  create_scene_function = LoadCreateSceneFunction(scene_dir)
  RenderSceneInWindow(window, create_scene_function)
end

end
