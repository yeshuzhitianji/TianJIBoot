from pathlib import Path
from PIL import Image
import shutil
import os
# 获取用户输入
while True:
    try:
        目标宽度 = int(input('请输入背景宽度（屏幕像素宽度）：'))
        目标高度 = int(input('请输入背景高度（屏幕像素高度）：'))
        if 目标宽度 > 0 and 目标高度 > 0:
            break
        print('输入值必须大于0，请重新输入！')
    except ValueError:
        print('请输入有效的整数！')
# 获取基础路径
base_dir = Path(__file__).parent.resolve()

# 配置路径
src_dir = base_dir / 'bmp'
dst_dir = base_dir / 'Image'

# 创建目标目录
dst_dir.mkdir(exist_ok=True)

# 遍历bmp目录
for bmp_file in src_dir.glob('*.bmp'):
    try:
        # 构造新文件名
        new_name = f"{bmp_file.stem}.图片"
        dest_path = dst_dir / new_name
        
        # 复制文件并保留元数据
        shutil.copy2(bmp_file, dest_path)
        print(f'已复制: {bmp_file.name} -> {dest_path}')
        
    except Exception as e:
        print(f'处理文件 {bmp_file} 时出错: {str(e)}')

# 源目录和目标目录
src_dir = base_dir / '背景png'
dst_dir = base_dir / 'Image' / 'Background'

# 创建目标目录
dst_dir.mkdir(exist_ok=True)

for png_file in src_dir.glob('*.png'):
    try:
        # 单文件全流程处理
        with Image.open(png_file) as img:
            # 缩放阶段
            width_percent = 目标高度 / float(img.size[1])
            target_width = int(float(img.size[0]) * width_percent)
            resized_img = img.resize((target_width, 目标高度), Image.LANCZOS)
            
            # 裁剪阶段
            left = (resized_img.width - 目标宽度) // 2
            cropped_img = resized_img.crop((left, 0, left + 目标宽度, 目标高度))
            
            # 转换阶段
            rgba_img = cropped_img.convert('RGBA')
            
            # 命名并保存
            new_name = f"背景{png_file.stem}.图片"
            dest_path = dst_dir / new_name
            rgba_img.save(dest_path, 'BMP')
            
            print(f'已处理: {png_file.name} -> {dest_path}')
            
    except Exception as e:
        print(f'处理文件 {png_file} 时出错: {str(e)}')
