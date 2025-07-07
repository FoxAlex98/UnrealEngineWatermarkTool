import tkinter as tk
from tkinter import filedialog
from PIL import Image
import numpy as np
import matplotlib.pyplot as plt

def extract_invisible_watermark(image_path, watermark_size = None):
    """
    Extract Invisible Watermark From a PNG Image.
    You should use to check if an invisible Watermark is embedded in a Screenshot
    Args:
        image_path: watermarked PNG image path
        watermark_size: Tuple (width, height) watermark dimensions, if is None use image dimensions as watermark dimensions
    """
    
    img = Image.open(image_path)
    img_array = np.array(img)
    
    height, width = img_array.shape[:2]

    if(watermark_size == None):
        wm_width, wm_height = img.width, img.height
    else:
        (wm_width, wm_height) = watermark_size
    
    start_x = width - wm_width
    start_y = height - wm_height
    
    extracted_watermark = np.zeros((wm_height, wm_width), dtype=np.uint8)
    
    for y in range(wm_height):
        for x in range(wm_width):
            target_x = start_x + x
            target_y = start_y + y
            
            if 0 <= target_x < width and 0 <= target_y < height:
                r_lsb = img_array[target_y, target_x, 0] & 1
                g_lsb = img_array[target_y, target_x, 1] & 1
                b_lsb = img_array[target_y, target_x, 2] & 1
                
                if (r_lsb + g_lsb + b_lsb) >= 2:
                    extracted_watermark[y, x] = 255
    
    watermark_image = Image.fromarray(extracted_watermark)
    return watermark_image

def main():
    
    root = tk.Tk()
    root.withdraw()
    image_path = filedialog.askopenfilename(
        title="Select Image PNG",
        filetypes=[("PNG files", "*.png")]
    )
    
    try:
        watermark = extract_invisible_watermark(image_path)
        
        plt.imshow(watermark)
        plt.axis('off')
        plt.title('Extracted Watermark')
        plt.show()
        
        #TODO: add save options
        
    except Exception as e:
        print(f"Error during watermark extraction: {str(e)}")

if __name__ == "__main__":
    main()