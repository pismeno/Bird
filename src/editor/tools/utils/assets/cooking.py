from pathlib import Path
from typing import BinaryIO
import shutil
import os
import ctypes
from ctypes import c_void_p, c_char_p, c_size_t, c_int

def stream_file(file_path: Path, out_file: BinaryIO) -> None:
    with open(file_path, "rb") as in_file:
        shutil.copyfileobj(in_file, out_file)


# Get the official shaderc DLL from your Vulkan SDK environment
vulkan_sdk = os.environ.get("VULKAN_SDK")
if not vulkan_sdk:
    raise RuntimeError("VULKAN_SDK environment variable not found.")

dll_path = Path(vulkan_sdk) / "Bin" / "shaderc_shared.dll"
if not dll_path.exists():
    raise FileNotFoundError(f"Could not find {dll_path}")

# Load the shaderc C library into Python memory
libshaderc = ctypes.CDLL(str(dll_path))

# Setup C function signatures
libshaderc.shaderc_compiler_initialize.restype = c_void_p
libshaderc.shaderc_compile_options_initialize.restype = c_void_p

libshaderc.shaderc_compile_options_set_target_env.argtypes = [c_void_p, c_int, c_int]
libshaderc.shaderc_compile_options_set_target_spirv.argtypes = [c_void_p, c_int]

libshaderc.shaderc_compile_into_spv.argtypes = [
    c_void_p, c_char_p, c_size_t, c_int, c_char_p, c_char_p, c_void_p
]
libshaderc.shaderc_compile_into_spv.restype = c_void_p

libshaderc.shaderc_result_get_compilation_status.argtypes = [c_void_p]
libshaderc.shaderc_result_get_compilation_status.restype = c_int

libshaderc.shaderc_result_get_error_message.argtypes = [c_void_p]
libshaderc.shaderc_result_get_error_message.restype = c_char_p

libshaderc.shaderc_result_get_length.argtypes = [c_void_p]
libshaderc.shaderc_result_get_length.restype = c_size_t

libshaderc.shaderc_result_get_bytes.argtypes = [c_void_p]
libshaderc.shaderc_result_get_bytes.restype = ctypes.POINTER(ctypes.c_char)

libshaderc.shaderc_result_release.argtypes = [c_void_p]
libshaderc.shaderc_compile_options_release.argtypes = [c_void_p]
libshaderc.shaderc_compiler_release.argtypes = [c_void_p]


def get_shaderc_kind(path: Path) -> int:
    """Maps file extensions to explicit shaderc_shader_kind enums."""
    name = path.name.lower()
    if name.endswith((".vert", ".vert.glsl")):
        return 0  # shaderc_glsl_vertex_shader
    elif name.endswith((".frag", ".frag.glsl")):
        return 1  # shaderc_glsl_fragment_shader
    elif name.endswith((".comp", ".comp.glsl")):
        return 2  # shaderc_glsl_compute_shader
    elif name.endswith((".geom", ".geom.glsl")):
        return 3  # shaderc_glsl_geometry_shader
    elif name.endswith((".tesc", ".tesc.glsl")):
        return 4  # shaderc_glsl_tess_control_shader
    elif name.endswith((".tese", ".tese.glsl")):
        return 5  # shaderc_glsl_tess_evaluation_shader
    
    return 14 # shaderc_glsl_infer_from_source (fallback)


def compile_glsl_to_spirv(file_path: Path, out_file: BinaryIO) -> None:
    """Compiles GLSL to SPIR-V natively using the official shaderc C API."""
    compiler = libshaderc.shaderc_compiler_initialize()
    options = libshaderc.shaderc_compile_options_initialize()
    
    # 0 = Vulkan, 4202496 = Vulkan 1.2 (from 1 << 22 | 2 << 12)
    libshaderc.shaderc_compile_options_set_target_env(options, 0, 4202496)
    # 66816 = SPIR-V 1.5 (from 0x010500)
    libshaderc.shaderc_compile_options_set_target_spirv(options, 66816)

    source = file_path.read_text(encoding="utf-8").encode("utf-8")
    filename = file_path.name.encode("utf-8")
    entry_point = b"main"
    
    shader_kind = get_shaderc_kind(file_path)
    
    result = libshaderc.shaderc_compile_into_spv(
        compiler, 
        source, 
        len(source), 
        shader_kind, 
        filename, 
        entry_point, 
        options
    )
    
    status = libshaderc.shaderc_result_get_compilation_status(result)
    
    if status != 0:
        error_msg = libshaderc.shaderc_result_get_error_message(result).decode("utf-8")
        libshaderc.shaderc_result_release(result)
        libshaderc.shaderc_compile_options_release(options)
        libshaderc.shaderc_compiler_release(compiler)
        raise RuntimeError(f"GLSL compilation failed for {file_path.name}:\n{error_msg}")
        
    length = libshaderc.shaderc_result_get_length(result)
    bytes_ptr = libshaderc.shaderc_result_get_bytes(result)
    
    # Extract the raw SPIR-V binary bytes
    spirv_bytes = ctypes.string_at(bytes_ptr, length)
    
    # Free native shaderc resources
    libshaderc.shaderc_result_release(result)
    libshaderc.shaderc_compile_options_release(options)
    libshaderc.shaderc_compiler_release(compiler)
    
    # Write the compiled bytes to the provided stream
    out_file.write(spirv_bytes)