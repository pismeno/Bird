from pathlib import Path
from collections.abc import Callable
import argparse
import json

from utils.assets.meta_utils import get_meta_file_path
from utils.assets import cooking

parser = argparse.ArgumentParser(
    description="Batch process text or log files."
)

parser.add_argument(
    "destination",
    type=Path,
    help="Destination folder for the cooked assets"
)

parser.add_argument(
    "files",
    nargs="+",
    type=Path,
    help="One or more file paths to process",
)

args = parser.parse_args()

cook_funcs: dict[str, Callable[[Path, Path], None]] = {
    "text_asset" : cooking.copy_file,
    "image_asset" : cooking.copy_file
}

for file_path_str in args.files:
    file_path = Path(file_path_str)
    meta_file_path = get_meta_file_path(file_path)

    if not file_path.is_file():
        print(str(file_path) + " is not a file")
    if not meta_file_path.is_file():
        print(str(file_path) + " does not have a .meta file")

    with open(meta_file_path, "r") as mf:
        mf_json = json.loads(mf.read())
        for cooked_asset in mf_json["cooked_assets"]:
            dst_path = args.destination / (cooked_asset['uuid'] + ".asset")
            cook_funcs[cooked_asset["type"]](file_path, dst_path)