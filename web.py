
import os
from socket import socket
from time import time, sleep
from time import time
from typing import Container
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions
from selenium.common.exceptions import NoSuchElementException, TimeoutException
from selenium.webdriver.common.action_chains import ActionChains

class URL:
    def __init__(self) -> None:
        self.phpmyadmin = 'http://140.124.42.65/phpmyadmin'
        self.DHEMS_web_loadFix = 'http://140.124.42.65/how/DHEMS_web/loadFix.html'
        self.DHEMS_web_index = 'http://140.124.42.65/how/DHEMS_web/index.html'
        self.DHEMS_web_baseParameter = 'http://140.124.42.65/how/DHEMS_web/baseParameter.html'

class Xpath:
    def __init__(self) -> None:
        self.input_username = '//*[@id="input_username"]'
        self.input_passowrd = '//*[@id="input_password"]'
        self.btn_login = '//*[@id="input_go"]'
        self.btn_DHEMS_group = '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[1]/a/img'
        self.text_DB_DHMES_fiftyHousehold = '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/a'
        self.alert_loading = '//*[@id="pma_navigation_header"]/img'
        # menu bar info
        self.serverinfo_DB = '//*[@id="serverinfo"]/a[2]'
        self.serverinfo_table = '//*[@id="serverinfo"]/a[3]'
        # DHEMS_fiftyHousehold tables
        self.text_create_newTable =                 '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[1]/a'
        self.text_backup_BaseParameter =            '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[2]/a'
        self.text_backup_GHEMS =                    '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[3]/a'
        self.text_backup_LHEMS =                    '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[4]/a'
        self.text_backup_totalLoad =                '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[5]/a'
        self.text_BaseParameter =                   '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[6]/a'
        self.text_cost =                            '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[7]/a'
        self.text_dr =                              '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[8]/a'
        self.text_dr_alpha =                        '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[9]/a'
        self.text_distributed_group =               '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[10]/a'
        self.text_GHEMS_control_status =            '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[11]/a'
        self.text_GHEMS_flag =                      '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[12]/a'
        self.text_GHEMS__history_control_status =   '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[13]/a'
        self.text_GHEMS_real_status =               '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[14]/a'
        self.text_GHEMS_variable =                  '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[15]/a'
        self.text_LHEMS_comfort_level =             '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[16]/a'
        self.text_LHEMS_control_status =            '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[17]/a'
        self.text_LHEMS_dr_participation =          '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[18]/a'
        self.text_LHEMS_flag =                      '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[19]/a'
        self.text_LHEMS__history_control_status =   '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[20]/a'
        self.text_LHEMS_real_status =               '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[21]/a'
        self.text_LHEMS_ucLoad =                    '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[22]/a'
        self.text_load_list =                       '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[23]/a'
        self.text_price =                           '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[24]/a'
        self.text_soalr_data =                      '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[25]/a'
        self.text_soalr_day =                       '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[26]/a'
        self.text_totalLoad_model =                 '//*[@id="pma_navigation_tree_content"]/ul/li[3]/div[3]/ul/li[4]/div[4]/ul/li[27]/a'
        # page export
        self.btn_export = '//*[@id="topmenu"]/li[6]/a'
        self.title_text_export = '//*[@id="header"]'
        self.select_type_export = '//*[@id="plugins"]'
        self.type_csv_export = '//*[@id="plugins"]/option[11]'
        self.submit_export = '//*[@id="buttonGo"]'

        self.breadcrumb = '//*[@id="breadcrumb"]/div/div/li[2]'
        self.btn_fiftyHousehold = '/html/body/div[2]/button[4]'
        self.btn_sweetalert = '/html/body/div[7]/div/div[6]/button[1]'

        self.GHEMS_Price = '//*[@id="priceVsLoad"]'
        self.GHEMS_SOC = '//*[@id="SOCVsLoad"]'
        self.GHEMS_loadModel = '//*[@id="loadModel"]'
        self.GHEMS_table = '/html/body/table'
        self.LHEMS_loadSum = '//*[@id="households_loadsSum"]'
        self.LHEMS_household_status = '//*[@id="each_household_status"]'
        self.LHEMS_load1 = '//*[@id="con_0"]'
        self.LHEMS_load2 = '//*[@id="con_1"]'
        self.LHEMS_load3 = '//*[@id="con_2"]'
        self.LHEMS_load4 = '//*[@id="con_3"]'
        self.LHEMS_load5 = '//*[@id="con_4"]'
        self.LHEMS_load5 = '//*[@id="con_5"]'
        self.LHEMS_load6 = '//*[@id="con_5"]'
        self.LHEMS_load7 = '//*[@id="con_6"]'
        self.LHEMS_load8 = '//*[@id="con_7"]'
        self.LHEMS_load9 = '//*[@id="con_8"]'
        self.LHEMS_load10 = '//*[@id="con_9"]'
        self.LHEMS_load11 = '//*[@id="con_10"]'
        self.LHEMS_load12 = '//*[@id="con_11"]'
        self.LHEMS_load13 = '//*[@id="con_12"]'
        self.LHEMS_load14 = '//*[@id="con_13"]'
        self.LHEMS_load15 = '//*[@id="con_14"]'

        self.baseParameter_table = '//*[@id="flag_table"]'
        self.baseParameter_table_SOCthresh = '//*[@id="SOCthres"]'
        self.baseParameter_table_dr_mode = '//*[@id="dr_mode"]'
        self.baseParameter_table_simulate_weather = '//*[@id="simulate_weather"]'
        self.baseParameter_table_simulate_price = '//*[@id="simulate_price"]'
        self.baseParameter_table_simulate_history_weather = '//*[@id="simulate_history_weather"]'

def webdriver_init(url, db_download_path = ""):
    options = Options()
    options.add_argument("--disable-notifications")  #不啟用通知
    options.add_experimental_option("excludeSwitches", ['enable-automation', 'ignore-certificate-errors']) # 關閉 "chrome目前受到自動測試軟體控制”信息"
    if "html" in url:
        options.add_argument("--start-fullscreen")
    else:
        prefs = {"download.default_directory" : db_download_path}
        options.add_experimental_option("prefs", prefs)

    chrome = webdriver.Chrome('./chromedriver', chrome_options=options)
    chrome.get(url)
    return chrome

def login(user_value, password_value):
    
    username = chrome.find_element_by_xpath(xpath.input_username)
    password = chrome.find_element_by_xpath(xpath.input_passowrd)
    username.send_keys(user_value)
    password.send_keys(password_value)
    login = chrome.find_element_by_xpath(xpath.btn_login)
    login.click()
    try:
        wait.until(expected_conditions.url_changes(url.phpmyadmin))
        status = True
    except TimeoutException:
        print("Element not visible after {0} seconds".format(timeout))
        status = False
    except Exception as e:
        print("An Exception Ocurred: {0}".format(e))
        status = "ERROR"
    return status

def choose_DHEMS_DB(DHEMS_group_btn_Xpath, fiftyHousehold_Xpath):
    
    DHEMS_group_btn = chrome.find_element_by_xpath(DHEMS_group_btn_Xpath)
    DHEMS_group_btn.click()
    try:
        DHEMS_fiftyHousehold = chrome.find_element_by_xpath(fiftyHousehold_Xpath)
        wait.until(expected_conditions.visibility_of(DHEMS_fiftyHousehold))
        DHEMS_fiftyHousehold.click()
        wait.until(expected_conditions.element_to_be_clickable((By.XPATH, xpath.text_create_newTable)))
        status = True
    except TimeoutException:
        print("Element not visible after {0} seconds".format(timeout))
        status = False
    except Exception as e:
        print("An Exception Ocurred: {0}".format(e))
        status = "ERROR"
    return status

def gotoTable(target):
    
    try:
        wait.until(expected_conditions.element_to_be_clickable((By.XPATH, target)))
        target = chrome.find_element_by_xpath(target)
        target.click()
        wait.until(expected_conditions.element_to_be_clickable((By.XPATH, xpath.serverinfo_table)))
        menubar_table = chrome.find_element_by_xpath(xpath.serverinfo_table)
        assert target.text in menubar_table.text
        status = True 
    except TimeoutException:
        print("Element not visible after {0} seconds".format(timeout))
        status = False
    except Exception as e:
        print("An Exception Ocurred: {0}".format(e))
        status = "ERROR"
    return status

def gotoExport(table_name, default_type = 'CSV'):
    
    wait.until(expected_conditions.invisibility_of_element(chrome.find_element_by_xpath(xpath.alert_loading)))
    wait.until(expected_conditions.element_to_be_clickable((By.XPATH, xpath.btn_export)))
    chrome.find_element_by_xpath(xpath.btn_export).click()
    try:
        wait.until(expected_conditions.presence_of_element_located((By.XPATH, xpath.title_text_export)))
        title = chrome.find_element_by_xpath(xpath.title_text_export)
        table_name = chrome.find_element_by_xpath(table_name)
        assert table_name.text in title.text

        chrome.find_element_by_xpath(xpath.select_type_export).click()
        wait.until(expected_conditions.element_to_be_clickable((By.XPATH, xpath.type_csv_export)))
        type = chrome.find_element_by_xpath(xpath.type_csv_export)
        assert type.text == default_type
        type.click()
        chrome.find_element_by_xpath(xpath.submit_export).click()
        status = True
    except TimeoutException:
        print("Element not visible after {0} seconds".format(timeout))
        status = False
    except Exception as e:
        print("An Exception Ocurred: {0}".format(e))
        status = "ERROR"
    return status

def screenshot_file(screenshot_path, file):
    offset = 79
    sleep(1)
    if "GHEMS" in file:
        if chrome.current_url != url.DHEMS_web_loadFix:
            chrome.get(url.DHEMS_web_loadFix)        
        if "_Price" in file:
            element = chrome.find_element_by_xpath(xpath.GHEMS_Price)
        elif "_SOC" in file:
            element = chrome.find_element_by_xpath(xpath.GHEMS_SOC)
        elif "_loadModel" in file:
            element = chrome.find_element_by_xpath(xpath.GHEMS_loadModel)
        elif "_table" in file:
            element = chrome.find_element_by_xpath(xpath.GHEMS_table)
        chrome.execute_script("document.documentElement.scrollTop="+str(element.location['y']-offset))
    elif "LHEMS" in file:
        chrome.get(url.DHEMS_web_index)
        element = chrome.find_element_by_xpath(xpath.LHEMS_loadSum)
        chrome.execute_script("document.documentElement.scrollTop="+str(element.location['y']-offset))
    sleep(1)
    element.screenshot(screenshot_path + file)
    sleep(0.5)     

def choose_DB_by_btn(DB_name):
    sleep(1)
    webinfo = chrome.find_element_by_xpath(xpath.breadcrumb).text
    if DB_name not in webinfo:
        chrome.get(url.DHEMS_web_baseParameter)
        chrome.find_element_by_xpath(xpath.btn_fiftyHousehold).click()
        wait.until(expected_conditions.element_to_be_clickable((By.XPATH, xpath.btn_sweetalert)))
        chrome.find_element_by_xpath(xpath.btn_sweetalert).click()
        sleep(1)

def setting_screenshot_path(target_folder, fix_path = "C:\\Users\\sonu\\Desktop\\howThesis\\HEMSresult\\"):
    screenshot_path = fix_path + target_folder
    chrome.execute_script("document.documentElement.scrollTop=10000")
    chrome.find_element_by_xpath(xpath.baseParameter_table).click()
    chrome.execute_script("document.documentElement.scrollTop=10000")
    
    SOC_threshold = chrome.find_element_by_xpath(xpath.baseParameter_table_SOCthresh).get_attribute("value")
    dr_mode = chrome.find_element_by_xpath(xpath.baseParameter_table_dr_mode).get_attribute("value")
    price = chrome.find_element_by_xpath(xpath.baseParameter_table_simulate_price).get_attribute("value")
    weather = chrome.find_element_by_xpath(xpath.baseParameter_table_simulate_weather).get_attribute("value")
    price += "\\"
    weather += "\\"
    # 2021/09/07 don't consider history weather
    # example: "C:\\Users\\sonu\\Desktop\\howThesis\\HEMSresult\\9.comfortLevel\\summer_price\\sunny\\SOCinit0.3_dr1\\"
    # example: "C:\\Users\\sonu\\Desktop\\howThesis\\HEMSresult\\9.comfortLevel\\summer_price\\sunny\\SOCinit0.3\\"
    if int(dr_mode) != 0:
        SOC_threshold = "SOCinit" + SOC_threshold + "_dr" + dr_mode + "\\"    
    else:
        SOC_threshold = "SOCinit" + SOC_threshold + "\\"
    screenshot_path = screenshot_path + price + weather + SOC_threshold

    # example: "C:\\Users\\sonu\\Desktop\\howThesis\\HEMSresult\\7.50household\\not_summer_price\\sunny\\SOCinit0.7_dr1\\sunny\\"
    # if int(dr_mode) != 0:
    #     history_weather = chrome.find_element_by_xpath(xpath.baseParameter_table_simulate_history_weather).get_attribute("value")
    #     history_weather += "\\"
    #     SOC_threshold = "SOCinit" + SOC_threshold + "_dr" + dr_mode + "\\"
    #     screenshot_path = screenshot_path + price + history_weather + SOC_threshold + weather
    # else:
    #     SOC_threshold = "SOCinit" + SOC_threshold + "\\"
    #     screenshot_path = screenshot_path + price + weather + SOC_threshold

    try:
        os.makedirs(screenshot_path)
    except FileExistsError:
        pass
    return screenshot_path, dr_mode

if __name__ == "__main__":
    
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
    
    # '''
    # =-=-=-=-=-=- Get GHEMS and LHEMS screen shot -=-=-=-=-=-=
    chrome = webdriver_init(url.DHEMS_web_loadFix)
    wait = WebDriverWait(chrome, timeout=30)
    choose_DB_by_btn("DHEMS_fiftyHousehold")
    screenshot_path, dr_mode = setting_screenshot_path("9.comfortLevel\\")
    print("\n=-=-=-=-=-=-=-=-=-=")
    print("file import to ===> ", screenshot_path)
    print("=-=-=-=-=-=-=-=-=-=\n")
    screenshot_file(screenshot_path, "LHEMS.jpg")
    screenshot_file(screenshot_path, "GHEMS_Price.jpg")
    screenshot_file(screenshot_path, "GHEMS_SOC.jpg")
    screenshot_file(screenshot_path, "GHEMS_loadModel.jpg")
    screenshot_file(screenshot_path, "GHEMS_table.jpg")
    chrome.close()
    
    # '''
    # =-=-=-=-=-=- Export csv file from DB DHEMS_fiftyHousehold -=-=-=-=-=-=
    chrome = webdriver_init(url.phpmyadmin, screenshot_path)
    wait = WebDriverWait(chrome, timeout=30)
    db_user = "root"
    db_password = "fuzzy314"
    print("\n=-=-=-=-=-=-=-=-=-= login pass =-=-=-=-=-=-=-=-=-=") if login(db_user, db_password) else exit()
    print("=-=-=-=-=-=-=-=-=-= Go to DB pass =-=-=-=-=-=-=-=-=-=") if choose_DHEMS_DB(xpath.btn_DHEMS_group, xpath.text_DB_DHMES_fiftyHousehold) else exit()
    print("=-=-=-=-=-=-=-=-=-=")
    print("DB table import to ===> ", screenshot_path)
    print("=-=-=-=-=-=-=-=-=-=\n")
    export_tables = []
    export_tables.append(xpath.text_BaseParameter)
    export_tables.append(xpath.text_cost)
    if int(dr_mode) != 0:
        export_tables.append(xpath.text_dr_alpha)
    export_tables.append(xpath.text_GHEMS_control_status)
    export_tables.append(xpath.text_GHEMS_flag)
    export_tables.append(xpath.text_LHEMS_control_status)
    export_tables.append(xpath.text_LHEMS_flag)
    export_tables.append(xpath.text_totalLoad_model)

    for export_table in export_tables:
        gotoTable(export_table)
        sleep(0.5)
        gotoExport(export_table)
    sleep(1)
    chrome.close()
    # '''

