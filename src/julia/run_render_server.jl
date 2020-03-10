using ArgParse
using Sockets

function ArgParse.parse_item(::Type{IPAddr}, x::AbstractString)
    return parse(IPAddr, x)
end

function ParseCommandLine()
  arg_parse_settings = ArgParseSettings()
  arg_parse_settings.description =
"""
Starts a render server, see run_render_client.jl for instructions on starting
the client.
"""

  @add_arg_table arg_parse_settings begin
      "--password", "-p"
          help = "A shared secret between the server and clients.  Any string."
          arg_type = String
          required = true
      "--bind_address", "-a"
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

# We parse the command line first because this include statement can take
# some time to execute.
include("./render_server.jl")
using .RenderServer

StartSceneServer(parsed_args["password"],
                 bind_address=parsed_args["bind_address"],
                 port=parsed_args["port"])
