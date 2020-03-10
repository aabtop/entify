import os
import sys

import respire.buildlib.cc as cc
import respire.buildlib.modules as modules


def MakeProtobufLiteModule(registry, out_dir, configured_toolchain):
  return modules.StaticLibraryModule(
      'protobuf_lite', registry, out_dir, configured_toolchain,
      sources = [
        'src/google/protobuf/any_lite.cc',
        'src/google/protobuf/arena.cc',
        'src/google/protobuf/extension_set.cc',
        'src/google/protobuf/generated_enum_util.cc',
        'src/google/protobuf/generated_message_table_driven_lite.cc',
        'src/google/protobuf/generated_message_util.cc',
        'src/google/protobuf/implicit_weak_message.cc',
        'src/google/protobuf/io/coded_stream.cc',
        'src/google/protobuf/io/io_win32.cc',
        'src/google/protobuf/io/strtod.cc',
        'src/google/protobuf/io/zero_copy_stream.cc',
        'src/google/protobuf/io/zero_copy_stream_impl.cc',
        'src/google/protobuf/io/zero_copy_stream_impl_lite.cc',
        'src/google/protobuf/message_lite.cc',
        'src/google/protobuf/parse_context.cc',
        'src/google/protobuf/repeated_field.cc',
        'src/google/protobuf/stubs/bytestream.cc',
        'src/google/protobuf/stubs/common.cc',
        'src/google/protobuf/stubs/int128.cc',
        'src/google/protobuf/stubs/status.cc',
        'src/google/protobuf/stubs/statusor.cc',
        'src/google/protobuf/stubs/stringpiece.cc',
        'src/google/protobuf/stubs/stringprintf.cc',
        'src/google/protobuf/stubs/structurally_valid.cc',
        'src/google/protobuf/stubs/strutil.cc',
        'src/google/protobuf/stubs/time.cc',
        'src/google/protobuf/wire_format_lite.cc',
      ],
      public_include_paths=['src'])


def MakeProtobufModule(
    registry, out_dir, configured_toolchain, protobuf_lite_module):
  return modules.StaticLibraryModule(
      'protobuf', registry, out_dir, configured_toolchain,
      sources = [
        'src/google/protobuf/any.cc',
        'src/google/protobuf/any.pb.cc',
        'src/google/protobuf/api.pb.cc',
        'src/google/protobuf/compiler/importer.cc',
        'src/google/protobuf/compiler/parser.cc',
        'src/google/protobuf/descriptor.cc',
        'src/google/protobuf/descriptor.pb.cc',
        'src/google/protobuf/descriptor_database.cc',
        'src/google/protobuf/duration.pb.cc',
        'src/google/protobuf/dynamic_message.cc',
        'src/google/protobuf/empty.pb.cc',
        'src/google/protobuf/extension_set_heavy.cc',
        'src/google/protobuf/field_mask.pb.cc',
        'src/google/protobuf/generated_message_reflection.cc',
        'src/google/protobuf/generated_message_table_driven.cc',
        'src/google/protobuf/io/gzip_stream.cc',
        'src/google/protobuf/io/printer.cc',
        'src/google/protobuf/io/tokenizer.cc',
        'src/google/protobuf/map_field.cc',
        'src/google/protobuf/message.cc',
        'src/google/protobuf/reflection_ops.cc',
        'src/google/protobuf/service.cc',
        'src/google/protobuf/source_context.pb.cc',
        'src/google/protobuf/struct.pb.cc',
        'src/google/protobuf/stubs/substitute.cc',
        'src/google/protobuf/text_format.cc',
        'src/google/protobuf/timestamp.pb.cc',
        'src/google/protobuf/type.pb.cc',
        'src/google/protobuf/unknown_field_set.cc',
        'src/google/protobuf/util/delimited_message_util.cc',
        'src/google/protobuf/util/field_comparator.cc',
        'src/google/protobuf/util/field_mask_util.cc',
        'src/google/protobuf/util/internal/datapiece.cc',
        'src/google/protobuf/util/internal/default_value_objectwriter.cc',
        'src/google/protobuf/util/internal/error_listener.cc',
        'src/google/protobuf/util/internal/field_mask_utility.cc',
        'src/google/protobuf/util/internal/json_escaping.cc',
        'src/google/protobuf/util/internal/json_objectwriter.cc',
        'src/google/protobuf/util/internal/json_stream_parser.cc',
        'src/google/protobuf/util/internal/object_writer.cc',
        'src/google/protobuf/util/internal/proto_writer.cc',
        'src/google/protobuf/util/internal/protostream_objectsource.cc',
        'src/google/protobuf/util/internal/protostream_objectwriter.cc',
        'src/google/protobuf/util/internal/type_info.cc',
        'src/google/protobuf/util/internal/type_info_test_helper.cc',
        'src/google/protobuf/util/internal/utility.cc',
        'src/google/protobuf/util/json_util.cc',
        'src/google/protobuf/util/message_differencer.cc',
        'src/google/protobuf/util/time_util.cc',
        'src/google/protobuf/util/type_resolver_util.cc',
        'src/google/protobuf/wire_format.cc',
        'src/google/protobuf/wrappers.pb.cc',
      ],
      module_dependencies=[protobuf_lite_module],
      public_include_paths=['src'])


def MakeProtoCModule(registry, out_dir, configured_toolchain, protobuf_module):
  protoc_lib_module = modules.StaticLibraryModule(
      'protoc_lib', registry, out_dir, configured_toolchain,
      sources = [
        'src/google/protobuf/compiler/code_generator.cc',
        'src/google/protobuf/compiler/command_line_interface.cc',
        'src/google/protobuf/compiler/cpp/cpp_enum.cc',
        'src/google/protobuf/compiler/cpp/cpp_enum_field.cc',
        'src/google/protobuf/compiler/cpp/cpp_extension.cc',
        'src/google/protobuf/compiler/cpp/cpp_field.cc',
        'src/google/protobuf/compiler/cpp/cpp_file.cc',
        'src/google/protobuf/compiler/cpp/cpp_generator.cc',
        'src/google/protobuf/compiler/cpp/cpp_helpers.cc',
        'src/google/protobuf/compiler/cpp/cpp_map_field.cc',
        'src/google/protobuf/compiler/cpp/cpp_message.cc',
        'src/google/protobuf/compiler/cpp/cpp_message_field.cc',
        'src/google/protobuf/compiler/cpp/cpp_padding_optimizer.cc',
        'src/google/protobuf/compiler/cpp/cpp_primitive_field.cc',
        'src/google/protobuf/compiler/cpp/cpp_service.cc',
        'src/google/protobuf/compiler/cpp/cpp_string_field.cc',
        'src/google/protobuf/compiler/csharp/csharp_doc_comment.cc',
        'src/google/protobuf/compiler/csharp/csharp_enum.cc',
        'src/google/protobuf/compiler/csharp/csharp_enum_field.cc',
        'src/google/protobuf/compiler/csharp/csharp_field_base.cc',
        'src/google/protobuf/compiler/csharp/csharp_generator.cc',
        'src/google/protobuf/compiler/csharp/csharp_helpers.cc',
        'src/google/protobuf/compiler/csharp/csharp_map_field.cc',
        'src/google/protobuf/compiler/csharp/csharp_message.cc',
        'src/google/protobuf/compiler/csharp/csharp_message_field.cc',
        'src/google/protobuf/compiler/csharp/csharp_primitive_field.cc',
        'src/google/protobuf/compiler/csharp/csharp_reflection_class.cc',
        'src/google/protobuf/compiler/csharp/csharp_repeated_enum_field.cc',
        'src/google/protobuf/compiler/csharp/csharp_repeated_message_field.cc',
        'src/google/protobuf/compiler/csharp/csharp_repeated_primitive_field.cc',
        'src/google/protobuf/compiler/csharp/csharp_source_generator_base.cc',
        'src/google/protobuf/compiler/csharp/csharp_wrapper_field.cc',
        'src/google/protobuf/compiler/java/java_context.cc',
        'src/google/protobuf/compiler/java/java_doc_comment.cc',
        'src/google/protobuf/compiler/java/java_enum.cc',
        'src/google/protobuf/compiler/java/java_enum_field.cc',
        'src/google/protobuf/compiler/java/java_enum_field_lite.cc',
        'src/google/protobuf/compiler/java/java_enum_lite.cc',
        'src/google/protobuf/compiler/java/java_extension.cc',
        'src/google/protobuf/compiler/java/java_extension_lite.cc',
        'src/google/protobuf/compiler/java/java_field.cc',
        'src/google/protobuf/compiler/java/java_file.cc',
        'src/google/protobuf/compiler/java/java_generator.cc',
        'src/google/protobuf/compiler/java/java_generator_factory.cc',
        'src/google/protobuf/compiler/java/java_helpers.cc',
        'src/google/protobuf/compiler/java/java_map_field.cc',
        'src/google/protobuf/compiler/java/java_map_field_lite.cc',
        'src/google/protobuf/compiler/java/java_message.cc',
        'src/google/protobuf/compiler/java/java_message_builder.cc',
        'src/google/protobuf/compiler/java/java_message_builder_lite.cc',
        'src/google/protobuf/compiler/java/java_message_field.cc',
        'src/google/protobuf/compiler/java/java_message_field_lite.cc',
        'src/google/protobuf/compiler/java/java_message_lite.cc',
        'src/google/protobuf/compiler/java/java_name_resolver.cc',
        'src/google/protobuf/compiler/java/java_primitive_field.cc',
        'src/google/protobuf/compiler/java/java_primitive_field_lite.cc',
        'src/google/protobuf/compiler/java/java_service.cc',
        'src/google/protobuf/compiler/java/java_shared_code_generator.cc',
        'src/google/protobuf/compiler/java/java_string_field.cc',
        'src/google/protobuf/compiler/java/java_string_field_lite.cc',
        'src/google/protobuf/compiler/js/js_generator.cc',
        'src/google/protobuf/compiler/js/well_known_types_embed.cc',
        'src/google/protobuf/compiler/objectivec/objectivec_enum.cc',
        'src/google/protobuf/compiler/objectivec/objectivec_enum_field.cc',
        'src/google/protobuf/compiler/objectivec/objectivec_extension.cc',
        'src/google/protobuf/compiler/objectivec/objectivec_field.cc',
        'src/google/protobuf/compiler/objectivec/objectivec_file.cc',
        'src/google/protobuf/compiler/objectivec/objectivec_generator.cc',
        'src/google/protobuf/compiler/objectivec/objectivec_helpers.cc',
        'src/google/protobuf/compiler/objectivec/objectivec_map_field.cc',
        'src/google/protobuf/compiler/objectivec/objectivec_message.cc',
        'src/google/protobuf/compiler/objectivec/objectivec_message_field.cc',
        'src/google/protobuf/compiler/objectivec/objectivec_oneof.cc',
        'src/google/protobuf/compiler/objectivec/objectivec_primitive_field.cc',
        'src/google/protobuf/compiler/php/php_generator.cc',
        'src/google/protobuf/compiler/plugin.cc',
        'src/google/protobuf/compiler/plugin.pb.cc',
        'src/google/protobuf/compiler/python/python_generator.cc',
        'src/google/protobuf/compiler/ruby/ruby_generator.cc',
        'src/google/protobuf/compiler/subprocess.cc',
        'src/google/protobuf/compiler/zip_writer.cc',
      ],
      module_dependencies=[protobuf_module],
      public_include_paths=['src'])

  return modules.ExecutableModule(
      'protoc', registry, out_dir, configured_toolchain,
      sources = [
        'src/google/protobuf/compiler/main.cc',
      ],
      module_dependencies=[protoc_lib_module])


def ModifyConfigurationForProtobuf(configured_toolchain):
  WARNINGS_TO_DISABLE = [
    cc.Configuration.WARNING_SIGNED_UNSIGNED_MISMATCH,
    cc.Configuration.WARNING_POSSIBLE_LOSS_OF_DATA_WHEN_CONVERTING,
    cc.Configuration.WARNING_NO_DEFINITION_FOR_INLINE_FUNCTION,
    cc.Configuration.WARNING_UNARY_MINUS_APPLIED_TO_UNSIGNED_TYPE,
    cc.Configuration.WARNING_SWITCH_STATEMENT_CONTAINS_ONLY_DEFAULT,
    cc.Configuration.WARNING_FORCING_VALUE_TO_BOOL,
    cc.Configuration.WARNING_UNKNOWN_ATTRIBUTE,
    cc.Configuration.WARNING_IGNORE_MISSING_OVERRIDE,
  ]

  for warning in WARNINGS_TO_DISABLE:
    configured_toolchain.configuration.warning_toggles[warning] = False  

  # Only Windows and pthread platforms are supported currently, and all
  # HAVE_PTHREAD checks are done after Win32 checks.  
  configured_toolchain.configuration.defines += ['HAVE_PTHREAD']

  return configured_toolchain


def Build(registry, out_dir, configured_toolchain, host_configured_toolchain):
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

  host_out_dir = os.path.join(out_dir, 'host')
  if not os.path.exists(host_out_dir):
    os.makedirs(host_out_dir)

  protobuf_modules = {}

  configured_toolchain = ModifyConfigurationForProtobuf(configured_toolchain)
  host_configured_toolchain = ModifyConfigurationForProtobuf(
      host_configured_toolchain)

  # Build the protobuf runtime library using the passed in target toolchain.
  protobuf_modules['protobuf_lite'] = MakeProtobufLiteModule(
      registry, out_dir, configured_toolchain)
  protobuf_modules['protobuf'] = MakeProtobufModule(
      registry, out_dir, configured_toolchain,
      protobuf_modules['protobuf_lite'])

  # Now build protoc using the host toolchain.
  host_protobuf_lite = MakeProtobufLiteModule(
      registry, host_out_dir, host_configured_toolchain)
  host_protobuf = MakeProtobufModule(
      registry, host_out_dir, host_configured_toolchain, host_protobuf_lite)

  protobuf_modules['protoc'] = MakeProtoCModule(
      registry, host_out_dir, host_configured_toolchain, host_protobuf)

  return protobuf_modules
