module RenderClient

include("./server_security.jl")
using .ServerSecurity

import Base.Filesystem
import HTTP
import MbedTLS
import ZipFile

using Sockets

export SendScene

function SendScene(
    password::String, scene_dir_path::String; host::IPAddr=Sockets.localhost,
    port::Integer=4851)
  if !isdir(scene_dir_path)
    error("Expected directory path is not a directory: " * scene_dir_path)
  end

  (temp_filename, temp_iostream) = Filesystem.mktemp()
  
  zip_writer = ZipFile.Writer(temp_iostream)

  abs_scene_dir_path = Filesystem.abspath(scene_dir_path)

  for (root, dirs, files) in walkdir(abs_scene_dir_path)
    for file in files
      abs_file_path = Filesystem.joinpath(root, file)
      file_rel_root = Filesystem.relpath(abs_file_path, abs_scene_dir_path)
      
      zip_file = ZipFile.addfile(
          zip_writer, file_rel_root, method=ZipFile.Deflate)
      open(abs_file_path) do file
        write(zip_file, file)
      end
    end
    close(zip_writer)
    close(temp_iostream)

    println("ZipFile path: " * temp_filename)
    url = "http://" * string(host) * ":" * string(port)
    println("Send to URL: " * url)
    open(temp_filename) do file
      response = undef
      try
        response = SendEncryptedMessage(
            password, url * "/nonce", url * "/message", read(file))
        println("Success.")
      catch e
        if e isa HTTP.ExceptionRequest.StatusError
          @error(e)
        else
          rethrow(e)
        end
      end
    end
  end
end

end
