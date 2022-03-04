import os
from time import time
from time import sleep
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions
from selenium.common.exceptions import TimeoutException
from utils import URL, Xpath
from enum import Enum
import shutil

class WEBDRIVER:
    def __init__(self, url, screenshot_path = "") -> None:
        self.offset=79
        self.timeout=30
        self.fix_path="C:\\Users\\sonu\\Desktop\\howThesis\\HEMSresult\\"
        self.screenshot_path=self.fix_path+screenshot_path
        self.DHEMS="DHEMS"
        self.DHEMS_dr1="DHEMS_dr1"
        self.DHEMS_dr2="DHEMS_dr2"
        self.DHEMSFifty="DHEMS_FiftyHousehold"
        self.user_value="root"
        self.password_value="fuzzy314"
        self.EV_flag=""
        self.EM_flag=""
        self.nonEmpty_table_array=[]
        self.backupTable_array = {
            'rowCount': [
                xpath.rowCount_backup_BaseParameter, xpath.rowCount_backup_EM_Parameter, xpath.rowCount_backup_EM_user_number, 
                xpath.rowCount_backup_EM_user_result, xpath.rowCount_backup_EV_Parameter, xpath.rowCount_backup_EV_user_number, 
                xpath.rowCount_backup_EV_user_result, xpath.rowCount_backup_GHEMS, xpath.rowCount_backup_GHEMS_ucLoad, 
                xpath.rowCount_backup_LHEMS, xpath.rowCount_backup_LHEMS_cost, xpath.rowCount_backup_totalLoad
            ],
            'table': [
                xpath.text_backup_BaseParameter, xpath.text_backup_EM_Parameter, xpath.text_backup_EM_user_number, 
                xpath.text_backup_EM_user_result, xpath.text_backup_EV_Parameter, xpath.text_backup_EV_user_number, 
                xpath.text_backup_EV_user_result, xpath.text_backup_GHEMS, xpath.text_backup_GHEMS_ucLoad, 
                xpath.text_backup_LHEMS, xpath.text_backup_LHEMS_cost, xpath.text_backup_totalLoad
            ],    
            'name': [
                xpath.str_BaseParameter, xpath.str_EM_Parameter, xpath.str_EM_user_number, 
                xpath.str_EM_user_result, xpath.str_EV_Parameter, xpath.str_EV_user_number, 
                xpath.str_EV_user_result, xpath.str_GHEMS_control_status, xpath.str_GHEMS_ucLoad, 
                xpath.str_LHEMS_control_status, xpath.str_LHEMS_cost, xpath.str_totalLoad_model
            ],        
        }
        # setting driver then open browser
        options = Options()
        options.add_argument('--headless')
        options.add_argument("--disable-notifications")  #不啟用通知
        prefs = {
            "download.default_directory" : self.screenshot_path, 
            "credentials_enable_service": False, 
            "profile.password_manager_enabled": False,
        }
        options.add_experimental_option("excludeSwitches", ['enable-automation', 'ignore-certificate-errors']) # 關閉 "chrome目前受到自動測試軟體控制”信息"
        options.add_experimental_option("prefs", prefs)
        self.chrome = webdriver.Chrome('C:/Users/sonu/Desktop/howThesis/webDriver/chromedriver', options=options)
        self.chrome.set_window_size(1920, 1080)
        self.chrome.get(url)
        self.wait = WebDriverWait(self.chrome, timeout=self.timeout)

    def __del__(self):
        self.chrome.close()

    def loginPHPMyAdmin(self):
        username = self.chrome.find_element_by_xpath(xpath.input_username)
        password = self.chrome.find_element_by_xpath(xpath.input_passowrd)
        username.send_keys(self.user_value)
        password.send_keys(self.password_value)
        login = self.chrome.find_element_by_xpath(xpath.btn_login)
        login.click()
        try:
            self.wait.until(expected_conditions.url_changes(url.phpmyadmin))
            print(f"=-=-=-=-=-=-=-=-=-= Success login DB =-=-=-=-=-=-=-=-=-=")
        except TimeoutException:
            print(f"Element not visible after {self.timeout} seconds")
        except Exception as e:
            print(f"An Exception Ocurred: {format(e)}")

    def choose_DHEMS_DB(self, DHEMS_group_btn_Xpath, fiftyHousehold_Xpath):
        # expand and collapse DHEMS group
        DHEMS_group_btn = self.chrome.find_element_by_xpath(DHEMS_group_btn_Xpath)
        DHEMS_group_btn.click()
        try:
            DHEMS_fiftyHousehold = self.chrome.find_element_by_xpath(fiftyHousehold_Xpath)
            self.wait.until(expected_conditions.visibility_of(DHEMS_fiftyHousehold))
            DHEMS_fiftyHousehold.click()
            self.wait.until(expected_conditions.element_to_be_clickable((By.XPATH, xpath.text_create_newTable)))
            print(f"=-=-=-=-=-=-=-=-=-= Success expand DB table =-=-=-=-=-=-=-=-=-=")
            print(f"=-=-=-=-=-=-=-=-=-= DB table import to ===> {self.screenshot_path} =-=-=-=-=-=-=-=-=-=")
        except TimeoutException:
            print(f"Element not visible after {format(self.timeout)} seconds")
        except Exception as e:
            print(f"An Exception Ocurred: {format(e)}")

    def checkBackupRowCountIsZero(self):        
        try:
            # if self.chrome.find_element_by_xpath(xpath.rowCount_backup_BaseParameter):
            self.nonEmpty_table_array=self.backupTable_array["table"].copy()
            target_array2 = self.backupTable_array["rowCount"].copy()
            for target_table in self.backupTable_array['rowCount']:
                if self.chrome.find_element_by_xpath(target_table).text == '0':
                    self.nonEmpty_table_array.pop(target_array2.index(target_table))
                    target_array2.pop(target_array2.index(target_table))
            return len(self.nonEmpty_table_array)
        except TimeoutException:
            print(f"Element not visible after {format(self.timeout)} seconds")

    def gotoTable(self, target):
        try:
            self.wait.until(expected_conditions.element_to_be_clickable((By.XPATH, target)))
            target = self.chrome.find_element_by_xpath(target)
            target.click()
            self.wait.until(expected_conditions.element_to_be_clickable((By.XPATH, xpath.serverinfo_table)))
            menubar_table = self.chrome.find_element_by_xpath(xpath.serverinfo_table)
            assert target.text in menubar_table.text
        except TimeoutException:
            print(f"Element not visible after {format(self.timeout)} seconds")
        except Exception as e:
            print(f"An Exception Ocurred: {format(e)}")

    def truncateTable(self):
        try:
            self.chrome.find_element_by_xpath(xpath.btn_operate).click()
            self.wait.until(expected_conditions.element_to_be_clickable((By.XPATH, xpath.text_truncate_table)))
            self.chrome.find_element_by_xpath(xpath.text_truncate_table).click()
            self.wait.until(expected_conditions.element_to_be_clickable((By.XPATH, xpath.btn_submit_truncate)))
            self.chrome.find_element_by_xpath(xpath.btn_submit_truncate).click()
            sleep(1)
        except TimeoutException:
            print(f"Element not visible after {format(self.timeout)} seconds")
    
    def verifyBackupTablesIsEmpty(self):
        while self.checkBackupRowCountIsZero() != 0:
            for nonEmpty_table in self.nonEmpty_table_array:
                self.gotoTable(nonEmpty_table)
                self.truncateTable()
                self.chrome.find_element_by_xpath(xpath.text_DB_DHMES_fiftyHousehold).click()
                sleep(1)
        else:
            print(f"=-=-=-=-=-=-=-=-=-= All back up tables are empty =-=-=-=-=-=-=-=-=-=")

    def gotoImport(self, table_name):
        self.chrome.find_element_by_xpath(xpath.btn_import).click()
        self.wait.until(expected_conditions.element_to_be_clickable((By.XPATH, xpath.btn_upload_file)))
        upfile = self.chrome.find_element_by_xpath(xpath.btn_upload_file)
        upfile.send_keys(self.screenshot_path+table_name+".csv")
        sleep(0.5)
        self.chrome.execute_script("document.documentElement.scrollTop=10000")
        self.chrome.find_element_by_xpath(xpath.submit_export).click()

    def importTable(self):
        for import_name, import_table in zip(self.backupTable_array['name'], self.backupTable_array['table']):
            self.gotoTable(import_table)
            self.gotoImport(import_name)
            sleep(1)
            
    class Backup_img(Enum):
        RECOVER = 1
        SAVEAS = 2

    def re_screenshot_file(self, old_file_name, new_web_img, type:Backup_img.RECOVER):
        # scroll to corresponed img
        if "GHEMS" in old_file_name:
            if self.chrome.current_url != url.DHEMS_web_backup_GHEMS:
                self.chrome.get(url.DHEMS_web_backup_GHEMS)
            if "_Price" in old_file_name:
                element = self.chrome.find_element_by_xpath(xpath.GHEMS_Price)
            elif "_SOC" in old_file_name:
                element = self.chrome.find_element_by_xpath(xpath.GHEMS_SOC)
            elif "_loadModel" in old_file_name:
                element = self.chrome.find_element_by_xpath(xpath.GHEMS_loadModel)
            elif "_EMchargingSOC" in old_file_name:
                element = self.chrome.find_element_by_xpath(xpath.GHEMS_EMchargingSOC)
            elif "_EVchargingSOC" in old_file_name:
                element = self.chrome.find_element_by_xpath(xpath.GHEMS_EVchargingSOC)
            elif "_table" in old_file_name:
                element = self.chrome.find_element_by_xpath(xpath.GHEMS_table)
            self.chrome.execute_script("document.documentElement.scrollTop="+str(element.location['y']-self.offset))
        elif "LHEMS" in old_file_name:
            if self.chrome.current_url != url.DHEMS_web_backup_LHEMS:
                self.chrome.get(url.DHEMS_web_backup_LHEMS)
            element = self.chrome.find_element_by_xpath(xpath.LHEMS_loadSum)
            self.chrome.execute_script("document.documentElement.scrollTop="+str(element.location['y']-self.offset))
        
        fileExist = os.path.isfile(self.screenshot_path+old_file_name)
        imgExist = self.chrome.find_element_by_xpath(new_web_img).size
        if fileExist and (imgExist['height']!=0) and (imgExist['width']!=0):            
            if type == type.RECOVER:
                os.remove(self.screenshot_path+old_file_name)
            elif type == type.SAVEAS:
                old_file_name=old_file_name[:-4]+"(1)"+old_file_name[-4:]
        else:
            pass
        sleep(0.5)
        element.screenshot(self.screenshot_path+old_file_name)
        sleep(0.5)

    def re_screenshot_everyHousehold_eachLoad_file(self, type:Backup_img.RECOVER, householdTotal = 50):
        if self.chrome.current_url != url.DHEMS_web_backup_LHEMS:
            self.chrome.get(url.DHEMS_web_backup_LHEMS)
        bar = self.chrome.find_element_by_xpath(xpath.range_bar)
        self.wait.until(expected_conditions.visibility_of(bar))
        bar.click()
        self.chrome.find_element_by_xpath(xpath.backup_range_bar_go).click()

        chart_sequence = {
            'file_name' : ["cost.jpg", "status.jpg", "1.jpg", "2.jpg", "3.jpg", "4.jpg", "5.jpg", "6.jpg", "7.jpg", "8.jpg", "9.jpg", "10.jpg", "11.jpg", "12.jpg", "13.jpg", "14.jpg", "15.jpg"],
            'file_xpath' : [xpath.backup_LHEMS_table, xpath.LHEMS_household_status, xpath.LHEMS_load1, xpath.LHEMS_load2, xpath.LHEMS_load3, xpath.LHEMS_load4, xpath.LHEMS_load5, xpath.LHEMS_load6, xpath.LHEMS_load7, xpath.LHEMS_load8, xpath.LHEMS_load9, xpath.LHEMS_load10, xpath.LHEMS_load11, xpath.LHEMS_load12, xpath.LHEMS_load13, xpath.LHEMS_load14, xpath.LHEMS_load15]
        }
        chart_sequence['file_xpath'].reverse()
        chart_sequence['file_name'].reverse()
        for i in range (householdTotal):
            sleep(1)
            id = self.chrome.find_element_by_xpath(xpath.LHEMS_household_text).get_attribute("value")
            if int(id) == i+1:
                print(f"Doing household id {id}")
                dirExist = os.path.isdir(self.screenshot_path+id)
                if dirExist:
                    if type == type.RECOVER:
                        shutil.rmtree(self.screenshot_path + id)
                    elif type == type.SAVEAS:
                        id += "(1)"
                else:
                    pass
                id += "\\"
                # create household num folder
                try:
                    os.makedirs(self.screenshot_path + id)
                except FileExistsError:
                    pass
                for file_xpath, file_name in zip(chart_sequence['file_xpath'], chart_sequence['file_name']):
                    element = self.chrome.find_element_by_xpath(file_xpath)
                    self.chrome.execute_script("document.documentElement.scrollTop="+str(element.location['y']-self.offset))
                    element.screenshot(self.screenshot_path + id + file_name)
                # go to page bottom to click next household button
                self.chrome.execute_script("document.documentElement.scrollTop=10000")
                self.chrome.find_element_by_xpath(xpath.LHEMS_nextHousehold_btn).click()
            else:
                print(f"Choosing Household {id} is not same as the target Household {i+1}, go out function")
                return

if __name__ == "__main__":
    
    url = URL()
    xpath = Xpath()
    screenshot_path=r"12.EMEV\both_discharging\grid500_ess150rate0.7\summer_price\cloudy\SOCinit0.3_dr5"
    screenshot_path += "\\"
    db = WEBDRIVER(url=url.phpmyadmin, screenshot_path=screenshot_path)
    db.loginPHPMyAdmin()
    db.choose_DHEMS_DB(xpath.btn_DHEMS_group, xpath.text_DB_DHMES_fiftyHousehold)
    db.verifyBackupTablesIsEmpty()
    db.importTable()
    # go to backup LHEMS & GHEMS and screenshot
    # DEL old and save new => Backup_img.RECOVER; SAVE another new => Backup_img.SAVEAS
    webpage = WEBDRIVER(url=url.DHEMS_web_backup_GHEMS, screenshot_path=screenshot_path)
    webpage.re_screenshot_file("LHEMS.jpg", xpath.LHEMS_loadSum, webpage.Backup_img.RECOVER)
    webpage.re_screenshot_file("GHEMS_Price.jpg", xpath.GHEMS_Price, webpage.Backup_img.RECOVER)
    webpage.re_screenshot_file("GHEMS_SOC.jpg", xpath.GHEMS_SOC, webpage.Backup_img.RECOVER)
    webpage.re_screenshot_file("GHEMS_loadModel.jpg", xpath.GHEMS_loadModel, webpage.Backup_img.RECOVER)
    webpage.re_screenshot_file("GHEMS_EMchargingSOC.jpg", xpath.GHEMS_EMchargingSOC, webpage.Backup_img.RECOVER)
    webpage.re_screenshot_file("GHEMS_EVchargingSOC.jpg", xpath.GHEMS_EVchargingSOC, webpage.Backup_img.RECOVER)
    webpage.re_screenshot_file("GHEMS_table.jpg", xpath.GHEMS_table, webpage.Backup_img.RECOVER)
    webpage.re_screenshot_everyHousehold_eachLoad_file(webpage.Backup_img.RECOVER)