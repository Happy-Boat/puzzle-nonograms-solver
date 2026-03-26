from selenium import webdriver
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from webdriver_manager.chrome import ChromeDriverManager

def scrape_nonogram(url, output_file="puzzle.txt"):
    options = Options()
    options.add_argument("--headless")
    
    driver = webdriver.Chrome(service=Service(ChromeDriverManager().install()), options=options)
    
    driver.get(url)
    wait = WebDriverWait(driver, 20)
    wait.until(EC.presence_of_element_located((By.ID, "taskLeft")))

    row_container = driver.find_element(By.ID, "taskLeft")
    row_divs = row_container.find_elements(By.XPATH, "./div")
    row_clues = []
    for div in row_divs:
        nums = [n.text for n in div.find_elements(By.TAG_NAME, "span") if n.text.strip()]
        if not nums: 
            nums = div.text.strip().split()
        row_clues.append(nums)

    col_container = driver.find_element(By.ID, "taskTop")
    col_divs = col_container.find_elements(By.XPATH, "./div")
    col_clues = []
    for div in col_divs:
        nums = [n.text for n in div.find_elements(By.TAG_NAME, "span") if n.text.strip()]
        if not nums:
            nums = div.text.strip().split()
        col_clues.append(nums)

    with open(output_file, "w") as f:
        f.write(f"{len(row_clues)} {len(col_clues)}\n")
        
        f.write("ROWS\n")
        for cl in row_clues:
            f.write(" ".join(cl) + "\n")
            
        f.write("COLS\n")
        for cl in col_clues:
            f.write(" ".join(cl) + "\n")
            
    print(f"成功抓取谜题！数据已保存至 {output_file}")
    print(f"尺寸: {len(row_clues)}x{len(col_clues)}")

    driver.quit()

if __name__ == "__main__":
    target_url = "https://cn.puzzle-nonograms.com/?size=6"
    scrape_nonogram(target_url)