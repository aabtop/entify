# automatically generated by the FlatBuffers compiler, do not modify

MyGame.Example.eval(quote


FlatBuffers.@with_kw mutable struct Stat
    id::String = ""
    val::Int64 = 0
    count::UInt16 = 0
end
FlatBuffers.@ALIGN(Stat, 1)
FlatBuffers.slot_offsets(::Type{T}) where {T<:Stat} = [
    0x00000004, 0x00000006, 0x00000008
]

Stat(buf::AbstractVector{UInt8}) = FlatBuffers.read(Stat, buf)
Stat(io::IO) = FlatBuffers.deserialize(io, Stat)

end)
