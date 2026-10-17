from pathlib import Path
import argparse
import uuid
import json

import puremagic

from utils.assets import AssetType, get_meta_file_path

def append_type(processed_asset_list: list[dict], type: AssetType) -> None:
    processed_asset_list.append({
         "uuid" : str(uuid.uuid4()),
        "type" : str(type)
    })

parser = argparse.ArgumentParser(
    description="Batch process text or log files."
)

parser.add_argument(
    "files",
    nargs="+",
    type=Path,
    help="One or more file paths to process",
)

args = parser.parse_args()

for file_path_str in args.files:
    file_path = Path(file_path_str)
    try:
        mime_type = puremagic.from_file(str(file_path), mime=True)
    except puremagic.PureError:
        print("Could not identify file type for " + str(file_path))
        continue

    cooked_assets: list[dict] = []

    match mime_type.split("/"):        
        case ["image", "png" | "jpeg" | "jpg"]:
            append_type(cooked_assets, AssetType.IMAGE)
        case ["text", _] | ["application", "json"]:
            if file_path.name.endswith(".glsl"):
                file_name_ns = file_path.name.removesuffix(".glsl")
                if file_name_ns.endswith(".vert"):
                    append_type(cooked_assets, AssetType.VERTEX_SHADER)
                elif file_name_ns.endswith(".frag"):
                    append_type(cooked_assets, AssetType.FRAGMENT_SHADER)
                elif file_name_ns.endswith(".comp"):
                    append_type(cooked_assets, AssetType.COMPUTE_SHADER)
                elif file_name_ns.endswith(".geom"):
                    append_type(cooked_assets, AssetType.GEOMETRY_SHADER)
                elif file_name_ns.endswith(".tesc"):
                    append_type(cooked_assets, AssetType.TESS_CONTROL_SHADER)
                elif file_name_ns.endswith(".tese"):
                    append_type(cooked_assets, AssetType.TESS_EVALUATION_SHADER)
            else:            
                append_type(cooked_assets, AssetType.TEXT)
        case _:
            print("Unknown type by Mole engine")

    meta_content = { "cooked_assets" : cooked_assets }

    with open(get_meta_file_path(file_path), "w") as mf:
        mf.write(json.dumps(meta_content))