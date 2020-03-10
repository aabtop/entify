module Lib

using FlatBuffers

macro LibraryFunction(function_name_and_library, return_type, params)
  return :($(eval(function_name_and_library)[1])(
                 $(map(x -> x.args[1], params.args)...)) =
                 ccall($function_name_and_library, $return_type,
                       ($(map(x -> x.args[2], params.args)...),),
                       $(map(x -> x.args[1], params.args)...)))
end

macro PlatformWindowsLibraryFunction(function_name, return_type, params)
  full_function_name = Symbol("PlatformWindow", eval(function_name))
  platform_window_path =
      joinpath(splitdir(@__FILE__)[1], "platform_window")
  return :(@LibraryFunction(
               $(full_function_name, platform_window_path),
               $return_type, $params))
end

mutable struct PlatformWindow
end
mutable struct NativeWindow
end

@PlatformWindowsLibraryFunction(
    :MakeDefaultWindow, Ptr{PlatformWindow},
    (title::Cstring, event_callback::Ptr{Cvoid}, context::Ptr{Cvoid}))

@PlatformWindowsLibraryFunction(
    :DestroyWindow, Cvoid, (window::Ptr{PlatformWindow},))

@PlatformWindowsLibraryFunction(
    :GetNativeWindow, Ptr{NativeWindow}, (window::Ptr{PlatformWindow},))

@PlatformWindowsLibraryFunction(
    :GetWidth, Int32, (window::Ptr{PlatformWindow},))

@PlatformWindowsLibraryFunction(
    :GetHeight, Int32, (window::Ptr{PlatformWindow},))



macro EntifyLibraryFunction(function_name, return_type, params)
  full_function_name = Symbol("Entify", eval(function_name))
  platform_window_path =
      joinpath(splitdir(@__FILE__)[1], "entify")
  return :(@LibraryFunction(
               $(full_function_name, platform_window_path),
               $return_type, $params))
end

mutable struct EntifyContext
end
mutable struct EntifyReference
end
mutable struct EntifyRenderTarget
end

const EntifyId = Int64

@EntifyLibraryFunction(:CreateContext, Ptr{EntifyContext}, ())
@EntifyLibraryFunction(:DestroyContext, Cvoid, (context::Ptr{EntifyContext},))

@EntifyLibraryFunction(:TryGetReferenceFromId, Ptr{EntifyReference},
                       (context::Ptr{EntifyContext}, id::EntifyId))

@EntifyLibraryFunction(
    :CreateReferenceFromProtocolBuffer, Ptr{EntifyReference},
    (context::Ptr{EntifyContext}, id::EntifyId, data::Ref{UInt8},
     data_size::Csize_t))

@EntifyLibraryFunction(
    :CreateReferenceFromFlatBuffer, Ptr{EntifyReference},
    (context::Ptr{EntifyContext}, id::EntifyId, data::Ref{UInt8},
     data_size::Csize_t))

@EntifyLibraryFunction(
    :GetLastError, Int,
    (context::Ptr{EntifyContext}, message::Ref{Cstring}))

@EntifyLibraryFunction(
    :AddReference, Cvoid,
    (context::Ptr{EntifyContext}, reference::Ptr{EntifyReference}))
@EntifyLibraryFunction(
    :ReleaseReference, Cvoid,
    (context::Ptr{EntifyContext}, reference::Ptr{EntifyReference}))

@EntifyLibraryFunction(
    :CreateRenderTargetFromPlatformWindow,
    Ptr{EntifyRenderTarget},
    (context::Ptr{EntifyContext}, window::Ptr{NativeWindow},
    width::Int32, height::Int32))

@EntifyLibraryFunction(
    :ReleaseRenderTarget,
    Cvoid,
    (context::Ptr{EntifyContext}, render_target::Ptr{EntifyRenderTarget}))

@EntifyLibraryFunction(
    :Submit,
    Cvoid,
    (context::Ptr{EntifyContext}, render_tree::Ptr{EntifyReference},
     render_target::Ptr{EntifyRenderTarget}))


macro Blake2LibraryFunction(function_name, return_type, params)
  blake2_path = joinpath(splitdir(@__FILE__)[1], "blake2")
  return :(@LibraryFunction(
               $(Symbol(eval(function_name)), blake2_path),
               $return_type, $params))
end

@Blake2LibraryFunction(
    :blake2s, Cint, (out::Ref{UInt8}, in::Ptr{Cvoid}, key::Ptr{Cvoid},
    outlen::Csize_t, inlen::Csize_t, keylen::Csize_t))

end
