module References

using ..Entify

export ReferenceProvider, CreateReferenceNode, ReleaseReferenceNode,
       ReleaseAllReferences, UpdatableTextureReference, UpdateTexture!

mutable struct ReferenceProvider
  allocated_references::Set{Ptr{Entify.Lib.EntifyReference}}

  Acquire::Function
  Release::Function

  function ReferenceProvider(context::Ptr{Entify.Lib.EntifyContext})
    provider = new(Set{Ptr{Entify.Lib.EntifyReference}}())

    provider.Acquire = function(node::T) where {T <: Node}
      ref_node = ReferenceNode(context, node)
      push!(provider.allocated_references, ref_node.ref)
      return ref_node
    end

    provider.Release = function(ref_node::ReferenceNode)
      pop!(provider.allocated_references, ref_node.ref)
      Entify.Lib.EntifyReleaseReference(context, ref_node.ref)
    end

    return provider
  end
end

CreateReferenceNode(
    reference_provider::ReferenceProvider, node::T) where {T <: Node} =
    reference_provider.Acquire(node)

ReleaseReferenceNode(
    reference_provider::ReferenceProvider, reference_node::ReferenceNode) =
    reference_provider.Release(reference_node)

function ReleaseAllReferences(reference_provider::ReferenceProvider,
                              context::Ptr{Entify.Lib.EntifyContext})
  for reference in reference_provider.allocated_references
    Entify.Lib.EntifyReleaseReference(context, reference)
  end
  reference_provider.allocated_references =
      Set{Ptr{Entify.Lib.EntifyReference}}()
end

mutable struct UpdatableTextureReference
  reference_provider::ReferenceProvider
  texture::ReferenceNode{Texture}

  function UpdatableTextureReference(
      reference_provider::ReferenceProvider, initial_texture::Texture)
    return new(reference_provider,
               CreateReferenceNode(reference_provider, initial_texture))
  end
end

UpdateTexture!(updatable_texture_reference, new_content::DrawTree) =
    UpdateTexture!(
        updatable_texture_reference,
        RenderTarget(updatable_texture_reference.texture.width_in_pixels,
                     updatable_texture_reference.texture.height_in_pixels,
                     new_content))

UpdateTexture!(updatable_texture_reference, new_texture::Texture) =
    UpdateTexture!(
        updatable_texture_reference,
        CreateReferenceNode(updatable_texture_reference.reference_provider,
                            new_texture))

function UpdateTexture!(updatable_texture_reference::UpdatableTextureReference,
                        new_reference_node::ReferenceNode{Texture})
  ReleaseReferenceNode(
      updatable_texture_reference.reference_provider,
      updatable_texture_reference.texture)

  updatable_texture_reference.texture = new_reference_node
end

end
