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

class WEBDRIVER:
    def __init__(self, url, savingFolder="", screenshot_path = "") -> None:
        self.offset=79
        self.timeout=30
        self.screenshot_path=""
        self.DHEMS="DHEMS"
        self.DHEMS_dr1="DHEMS_dr1"
        self.DHEMS_dr2="DHEMS_dr2"
        self.DHEMSFifty="DHEMS_FiftyHousehold"
        self.user_value="root"
        self.password_value="fuzzy314"
        self.EV_flag=bool()
        self.EM_flag=bool()
        self.HEMS_ucLoad_flag=bool()
        self.comfortLevel_flag=bool()
        # setting driver then open browser
        options = Options()
        options.add_argument('--headless')
        options.add_argument("--disable-notifications")  #不啟用通知
        options.add_experimental_option("excludeSwitches", ['enable-automation', 'ignore-certificate-errors']) # 關閉 "chrome目前受到自動測試軟體控制”信息"
        if "html" in url:
            # options.add_argument("start-maximized")
            pass
        else:
            self.screenshot_path=screenshot_path
            prefs = {"download.default_directory" : self.screenshot_path}
            options.add_experimental_option("prefs", prefs)
        # headless can't work with "start-maximized" in the same time
        options.add_argument("--window-size=1920,1080")
        self.chrome = webdriver.Chrome('C:/Users/sonu/Desktop/howThesis/webDriver/chromedriver', options=options)
        self.chrome.get(url)
        self.wait = WebDriverWait(self.chrome, timeout=self.timeout)
        # get screenshot path
        if "html" in url:
            self.choose_DB_by_btn(self.DHEMSFifty)
            self.setting_screenshot_path(savingFolder)

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

    def gotoExport(self, table_name, default_type = 'CSV'):
        self.wait.until(expected_conditions.invisibility_of_element(self.chrome.find_element_by_xpath(xpath.alert_loading)))
        self.wait.until(expected_conditions.element_to_be_clickable((By.XPATH, xpath.btn_export)))
        self.chrome.find_element_by_xpath(xpath.btn_export).click()
        try:
            self.wait.until(expected_conditions.presence_of_element_located((By.XPATH, xpath.title_text_export)))
            title = self.chrome.find_element_by_xpath(xpath.title_text_export)
            table_name = self.chrome.find_element_by_xpath(table_name)
            assert table_name.text in title.text

            self.chrome.find_element_by_xpath(xpath.select_type_export).click()
            self.wait.until(expected_conditions.element_to_be_clickable((By.XPATH, xpath.type_csv_export)))
            type = self.chrome.find_element_by_xpath(xpath.type_csv_export)
            assert type.text == default_type
            type.click()
            self.chrome.find_element_by_xpath(xpath.submit_export).click()
        except TimeoutException:
            print(f"Element not visible after {format(self.timeout)} seconds")
        except Exception as e:
            print(f"An Exception Ocurred: {format(e)}")

    def exportTable(self, export_table):
        self.gotoTable(export_table)
        sleep(0.5)
        self.gotoExport(export_table)
        sleep(0.5)

    def screenshot_file(self, file):
        sleep(1)
        if "GHEMS" in file:
            if self.chrome.current_url != url.DHEMS_web_loadFix:
                self.chrome.get(url.DHEMS_web_loadFix)        
            if "_Price" in file:
                element = self.chrome.find_element_by_xpath(xpath.GHEMS_Price)
            elif "_SOC" in file:
                element = self.chrome.find_element_by_xpath(xpath.GHEMS_SOC)
            elif "_loadModel" in file:
                element = self.chrome.find_element_by_xpath(xpath.GHEMS_loadModel)
            elif "_EMchargingSOC" in file:
                element = self.chrome.find_element_by_xpath(xpath.GHEMS_EMchargingSOC)
            elif "_EVchargingSOC" in file:
                element = self.chrome.find_element_by_xpath(xpath.GHEMS_EVchargingSOC)
            elif "_table" in file:
                element = self.chrome.find_element_by_xpath(xpath.GHEMS_table)
            self.chrome.execute_script("document.documentElement.scrollTop="+str(element.location['y']-self.offset))
        elif "LHEMS" in file:
            self.chrome.get(url.DHEMS_web_index)
            element = self.chrome.find_element_by_xpath(xpath.LHEMS_loadSum)
            self.chrome.execute_script("document.documentElement.scrollTop="+str(element.location['y']-self.offset))
        sleep(1)
        element.screenshot(self.screenshot_path + file)
        sleep(0.5)

    def choose_DB_by_btn(self, DB_name):
        sleep(1)
        webinfo = self.chrome.find_element_by_xpath(xpath.breadcrumb).text
        if DB_name not in webinfo:
            self.chrome.get(url.DHEMS_web_baseParameter)
            self.chrome.find_element_by_xpath(xpath.btn_fiftyHousehold).click()
            self.wait.until(expected_conditions.element_to_be_clickable((By.XPATH, xpath.btn_sweetalert)))
            self.chrome.find_element_by_xpath(xpath.btn_sweetalert).click()
            sleep(1)

    def setting_screenshot_path(self, target_folder, fix_path = "C:\\Users\\sonu\\Desktop\\howThesis\\HEMSresult\\"):
        self.screenshot_path = fix_path + target_folder
        self.chrome.execute_script("document.documentElement.scrollTop=10000")
        self.chrome.find_element_by_xpath(xpath.baseParameter_table).click()
        self.chrome.execute_script("document.documentElement.scrollTop=10000")
        self.EM_flag = bool(int(self.chrome.find_element_by_xpath(xpath.baseParameter_table_EM_flag).get_attribute("value")))
        self.EV_flag = bool(int(self.chrome.find_element_by_xpath(xpath.baseParameter_table_EV_flag).get_attribute("value")))
        self.comfortLevel_flag = bool(int(self.chrome.find_element_by_xpath(xpath.baseParameter_table_comfortLevel_flag).get_attribute("value")))
        self.HEMS_ucLoad_flag = bool(int(self.chrome.find_element_by_xpath(xpath.baseParameter_table_HEMS_ucLoad_flag).get_attribute("value")))

        SOC_threshold = self.chrome.find_element_by_xpath(xpath.baseParameter_table_SOCthresh).get_attribute("value")
        dr_mode = self.chrome.find_element_by_xpath(xpath.baseParameter_table_dr_mode).get_attribute("value")
        price = self.chrome.find_element_by_xpath(xpath.baseParameter_table_simulate_price).get_attribute("value")
        weather = self.chrome.find_element_by_xpath(xpath.baseParameter_table_simulate_weather).get_attribute("value")
        # 2021/09/07 don't consider history weather
        # example: "C:\\Users\\sonu\\Desktop\\howThesis\\HEMSresult\\9.comfortLevel\\summer_price\\sunny\\{SOCinit0.3} or {SOCinit0.3_dr1}\\"
        if int(dr_mode) != 0:
            SOC_threshold = "SOCinit" + SOC_threshold + "_dr" + dr_mode + "\\"    
        else:
            SOC_threshold = "SOCinit" + SOC_threshold + "\\"
        
        folder_name = ""
        folder_name += "EM_" if self.EM_flag else "noEM_"
        folder_name += "EV_" if self.EV_flag else "noEV_"
        folder_name += "coml_" if self.comfortLevel_flag else "noComl_"
        folder_name += "HEMSuc" if self.HEMS_ucLoad_flag else "noHEMSuc"
        
        self.screenshot_path = self.screenshot_path + price + "\\" + folder_name + "\\" + weather + "\\" + SOC_threshold
        
        print(f"=-=-=-=-=-=-=-=-=-= File import to ===>  {self.screenshot_path} =-=-=-=-=-=-=-=-=-=")
        # example: "C:\\Users\\sonu\\Desktop\\howThesis\\HEMSresult\\7.50household\\not_summer_price\\sunny\\SOCinit0.7_dr1\\sunny\\"
        # if int(dr_mode) != 0:
        #     history_weather = self.chrome.find_element_by_xpath(xpath.baseParameter_table_simulate_history_weather).get_attribute("value")
        #     history_weather += "\\"
        #     SOC_threshold = "SOCinit" + SOC_threshold + "_dr" + dr_mode + "\\"
        #     screenshot_path = screenshot_path + price + history_weather + SOC_threshold + weather
        # else:
        #     SOC_threshold = "SOCinit" + SOC_threshold + "\\"
        #     screenshot_path = screenshot_path + price + weather + SOC_threshold
        try:
            os.makedirs(self.screenshot_path)
        except FileExistsError:
            pass

    def screenshot_everyHousehold_eachLoad_file(self, householdTotal = 50):
        if self.chrome.current_url != url.DHEMS_web_index:
            self.chrome.get(url.DHEMS_web_index)
        bar = self.chrome.find_element_by_xpath(xpath.range_bar)
        self.wait.until(expected_conditions.visibility_of(bar))
        bar.click()
        self.chrome.find_element_by_xpath(xpath.range_bar_go).click()

        chart_sequence = {
            'file_name' : ["cost.jpg", "status.jpg", "1.jpg", "2.jpg", "3.jpg", "4.jpg", "5.jpg", "6.jpg", "7.jpg", "8.jpg", "9.jpg", "10.jpg", "11.jpg", "12.jpg", "13.jpg", "14.jpg", "15.jpg", "16.jpg", "17.jpg", "18.jpg", "19.jpg", "20.jpg", "21.jpg", "22.jpg", "23.jpg"],
            'file_xpath' : [xpath.LHEMS_table, xpath.LHEMS_household_status, xpath.LHEMS_load1, xpath.LHEMS_load2, xpath.LHEMS_load3, xpath.LHEMS_load4, xpath.LHEMS_load5, xpath.LHEMS_load6, xpath.LHEMS_load7, xpath.LHEMS_load8, xpath.LHEMS_load9, xpath.LHEMS_load10, xpath.LHEMS_load11, xpath.LHEMS_load12, xpath.LHEMS_load13, xpath.LHEMS_load14, xpath.LHEMS_load15, xpath.LHEMS_load16, xpath.LHEMS_load17, xpath.LHEMS_load18, xpath.LHEMS_load19, xpath.LHEMS_load20, xpath.LHEMS_load21, xpath.LHEMS_load22, xpath.LHEMS_load23]
        }
        chart_sequence['file_xpath'].reverse()
        chart_sequence['file_name'].reverse()
        for i in range (householdTotal):
            sleep(1)
            id = self.chrome.find_element_by_xpath(xpath.LHEMS_household_text).get_attribute("value")
            if int(id) == i+1:
                print(f"Doing household id {id}")
                id += "\\"
                # create household num folder
                try:
                    os.makedirs(self.screenshot_path + id)
                except FileExistsError:
                    pass
                for file_xpath, file_name in zip(chart_sequence['file_xpath'], chart_sequence['file_name']):
                    element = self.chrome.find_element_by_xpath(file_xpath)
                    if "none" in element.get_attribute("style"):
                        pass
                    else:
                        self.chrome.execute_script("document.documentElement.scrollTop="+str(element.location['y']-self.offset))
                        element.screenshot(self.screenshot_path + id + file_name)
                # go to page bottom to click next household button
                self.chrome.execute_script("document.documentElement.scrollTop=10000")
                self.chrome.find_element_by_xpath(xpath.LHEMS_nextHousehold_btn).click()
            else:
                print(f"Choosing Household {id} is not same as the target Household {i+1}, go out function")
                return


if __name__ == "__main__":
    
    start_time=time()
    xpath = Xpath()
    url = URL()
    ###############################################################
    ## need to change:                                           ##
    ## Classes: Xpath & URL                                      ##
    ## functions:                                                ##
    ## setting_screenshot_path: 'target_folder' & default path   ##
    ## screenshot_file: name of the file                         ##
    ## choose_DHEMS_DB: 'DHEMS group' Process                    ##
    ###############################################################
    webpage = WEBDRIVER(url.DHEMS_web_baseParameter, savingFolder="13.EMEV_final\\")
    webpage.screenshot_file("LHEMS.jpg")
    webpage.screenshot_file("GHEMS_Price.jpg")
    webpage.screenshot_file("GHEMS_SOC.jpg")
    webpage.screenshot_file("GHEMS_loadModel.jpg")
    if webpage.EM_flag:
        webpage.screenshot_file("GHEMS_EMchargingSOC.jpg")
    if webpage.EV_flag:
        webpage.screenshot_file("GHEMS_EVchargingSOC.jpg")
    webpage.screenshot_file("GHEMS_table.jpg")
    webpage.screenshot_everyHousehold_eachLoad_file()

    db = WEBDRIVER(url=url.phpmyadmin, screenshot_path=webpage.screenshot_path)
    db.loginPHPMyAdmin()
    db.choose_DHEMS_DB(xpath.btn_DHEMS_group, xpath.text_DB_DHMES_fiftyHousehold)
    db.exportTable(xpath.text_cost)
    db.exportTable(xpath.text_BaseParameter)
    db.exportTable(xpath.text_GHEMS_control_status)
    db.exportTable(xpath.text_GHEMS_flag)
    db.exportTable(xpath.text_GHEMS_uncontrollable_load)
    db.exportTable(xpath.text_LHEMS_control_status)
    db.exportTable(xpath.text_LHEMS_flag)
    db.exportTable(xpath.text_LHEMS_cost)
    db.exportTable(xpath.text_totalLoad_model)
    # HEMS uc load
    if webpage.HEMS_ucLoad_flag:
        db.exportTable(xpath.text_LHEMS_ucLoad)
    # EM table
    if webpage.EM_flag:
        db.exportTable(xpath.text_EM_Parameter)
        db.exportTable(xpath.text_EM_user_number)
        db.exportTable(xpath.text_EM_user_result)
        db.exportTable(xpath.text_EM_chargingOrDischarging_status)
    # EV table
    if webpage.EV_flag:
        db.exportTable(xpath.text_EV_Parameter)
        db.exportTable(xpath.text_EV_user_number)
        db.exportTable(xpath.text_EV_user_result)
        db.exportTable(xpath.text_EV_chargingOrDischarging_status)
    print(f"//--------------- {time()-start_time} ---------------\\")