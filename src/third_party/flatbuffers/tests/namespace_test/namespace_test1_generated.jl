# automatically generated by the FlatBuffers compiler, do not modify

if !isdefined(@__MODULE__(), :NamespaceA) @__MODULE__().eval(:(module NamespaceA import FlatBuffers end)) end
if !isdefined(NamespaceA, :NamespaceB) NamespaceA.eval(:(module NamespaceB import FlatBuffers end)) end
include("NamespaceA/NamespaceB/EnumInNestedNS.jl")
include("NamespaceA/NamespaceB/TableInNestedNS.jl")
include("NamespaceA/NamespaceB/StructInNestedNS.jl")
