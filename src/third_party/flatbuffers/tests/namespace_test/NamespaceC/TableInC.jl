# automatically generated by the FlatBuffers compiler, do not modify

NamespaceC.eval(quote

import ..NamespaceA

FlatBuffers.@with_kw mutable struct TableInC
    refer_to_a1::Union{NamespaceA.TableInFirstNS, Nothing} = nothing
    refer_to_a2::Union{NamespaceA.SecondTableInA, Nothing} = nothing
end
FlatBuffers.@ALIGN(TableInC, 1)
FlatBuffers.slot_offsets(::Type{T}) where {T<:TableInC} = [
    0x00000004, 0x00000006
]

TableInC(buf::AbstractVector{UInt8}) = FlatBuffers.read(TableInC, buf)
TableInC(io::IO) = FlatBuffers.deserialize(io, TableInC)

end)

