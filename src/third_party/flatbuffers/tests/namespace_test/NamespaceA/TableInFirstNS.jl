# automatically generated by the FlatBuffers compiler, do not modify

NamespaceA.eval(quote

import .NamespaceB

FlatBuffers.@with_kw mutable struct TableInFirstNS
    foo_table::Union{NamespaceB.TableInNestedNS, Nothing} = nothing
    foo_enum::NamespaceB.EnumInNestedNS = 0
    foo_struct::Union{NamespaceB.StructInNestedNS, Nothing} = nothing
end
FlatBuffers.@ALIGN(TableInFirstNS, 1)
FlatBuffers.slot_offsets(::Type{T}) where {T<:TableInFirstNS} = [
    0x00000004, 0x00000006, 0x00000008
]

TableInFirstNS(buf::AbstractVector{UInt8}) = FlatBuffers.read(TableInFirstNS, buf)
TableInFirstNS(io::IO) = FlatBuffers.deserialize(io, TableInFirstNS)

end)

