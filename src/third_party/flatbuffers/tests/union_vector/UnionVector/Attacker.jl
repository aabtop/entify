# automatically generated by the FlatBuffers compiler, do not modify

UnionVector.eval(quote


FlatBuffers.@with_kw mutable struct Attacker
    sword_attack_damage::Int32 = 0
end
FlatBuffers.@ALIGN(Attacker, 1)
FlatBuffers.slot_offsets(::Type{T}) where {T<:Attacker} = [
    0x00000004
]

Attacker(buf::AbstractVector{UInt8}) = FlatBuffers.read(Attacker, buf)
Attacker(io::IO) = FlatBuffers.deserialize(io, Attacker)

end)

