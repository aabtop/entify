using ArgParse
using Sockets

include("./render_client.jl")
using .RenderClient

function ArgParse.parse_item(::Type{IPAddr}, x::AbstractString)
    return parse(IPAddr, x)
end


function ParseCommandLine()
  arg_parse_settings = ArgParseSettings()
  arg_parse_settings.description =
"""
Attempts to connect to a render server and send a new scene.
"""

  @add_arg_table arg_parse_settings begin
      "--password", "-p"
          help = "A shared secret between the server and clients.  Any string."
          arg_type = String
          required = true
      "--scene", "-s"
          help = "A path to a folder containing a main.jl which defines a " * 
                 "MakeDrawTree() function."
          arg_type = String
          required = true
      "--address", "-a"
          help = "The address the server should bind to for listening."
          arg_type = IPAddr
          default = ip"127.0.0.1"
      "--port"
          help = "The port that the server should listen on."
          arg_type = UInt16
          default = UInt16(4851)
  end

  return parse_args(arg_parse_settings)
end

parsed_args = ParseCommandLine()

SendScene(parsed_args["password"], parsed_args["scene"],
          host=parsed_args["address"], port=parsed_args["port"])
