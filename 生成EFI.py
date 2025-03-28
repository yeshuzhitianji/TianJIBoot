import subprocess
import shutil
import os
from pathlib import Path

# 基础路径配置
BASE_DIR = Path(__file__).parent.resolve()

# 源文件路径
PYTHON_SCRIPT = BASE_DIR / "天玑项目/python/生成图片.py"
# 动态选择EFI版本路径
release_path = BASE_DIR / "Build/天玑项目/RELEASE_GCC5/X64/天玑.efi"
debug_path = BASE_DIR / "Build/天玑项目/DEBUG_GCC5/X64/天玑.efi"

if release_path.exists():
    EFI_SOURCE = release_path
elif debug_path.exists():
    EFI_SOURCE = debug_path
else:
    raise FileNotFoundError("无法找到EFI文件：RELEASE和DEBUG版本均不存在，请运行build命令")

IMAGE_DIR = BASE_DIR / "天玑项目/python/Image"

# 目标路径
EFI_TARGET_DIR = BASE_DIR / "EFI/TianJI"
EFI_TARGET_FILE = EFI_TARGET_DIR / "天玑.中国"
IMAGE_TARGET = EFI_TARGET_DIR / "Image"

def run_python_script():
    try:
        print(f"正在执行脚本: {PYTHON_SCRIPT}")
        subprocess.run(['python', str(PYTHON_SCRIPT)], check=True)
        print("图片生成完成")
    except subprocess.CalledProcessError as e:
        print(f"脚本执行失败: {e}")
        raise

def copy_efi_file():
    try:
        EFI_TARGET_DIR.mkdir(parents=True, exist_ok=True)
        print(f"正在复制文件到: {EFI_TARGET_FILE}")
        shutil.copy2(str(EFI_SOURCE), str(EFI_TARGET_FILE))
        print("EFI文件复制完成")
    except Exception as e:
        print(f"文件复制失败: {e}")
        raise

def move_image_dir():
    try:
        EFI_TARGET_DIR.mkdir(parents=True, exist_ok=True)
        print(f"正在移动文件夹到: {IMAGE_TARGET}")
        if IMAGE_TARGET.exists():
            shutil.rmtree(str(IMAGE_TARGET))
        shutil.move(str(IMAGE_DIR), str(IMAGE_TARGET))
        print("Image文件夹移动完成")
    except Exception as e:
        print(f"文件夹移动失败: {e}")
        raise

if __name__ == "__main__":
    try:
        run_python_script()
        copy_efi_file()
        move_image_dir()
        print("所有操作执行完毕")
    except Exception as e:
        print(f"执行过程中发生错误: {e}")
        exit(1)