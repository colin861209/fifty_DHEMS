
import pandas as pd
import os
from itertools import product
import numpy as np

class StrPath:
    
    def __init__(self) -> None:
        self.__str_fixPath = r"C:\Users\sonu\Desktop\howThesis\HEMSresult"
        self.__dir_mainFolder = ['14.EMEV_new'] # main folder
        self.__dir_price = [
            'summer_price', 
            'comed_price', 
            ] # price
        self.__dir_comlv = [
            'EM_EV_coml_HEMSuc', 'EM_EV_noComl_HEMSuc', 
            'EM_noEV_coml_HEMSuc', 'EM_noEV_noComl_HEMSuc', 
            'noEM_EV_coml_HEMSuc', 'noEM_EV_noComl_HEMSuc', 
            'noEM_noEV_coml_HEMSuc', 'noEM_noEV_noComl_HEMSuc', 
            ] # comfort level
        self.__dir_weather= ['cloudy', 'sunny', ] # weather
        self.__dir_DR = ['SOCinit0.3_dr5', 'SOCinit0.3', ] # DR
        self.loop_list = [self.__dir_mainFolder, self.__dir_price, self.__dir_comlv, self.__dir_weather, self.__dir_DR]
        # 
        self.final_path = ''
        self.__output_path = 'FolderPath'
        self.__output_price = 'Price'
        self.__output_coml = 'ComfortLevel'
        self.__output_weather = 'Weather'
        self.__output_drmode = 'DR Mode'
        self.__output_maxConsumption = 'HEMS_MAX_Consumption(kW)'
        self.__output_publicLoad = 'publicLoad'
        self.__output_publicLoadSpend = 'publicLoadSpend'
        self.__output_LoadSpend = 'CEMS_Load_Spend(NTD)' # R^cost
        self.__output_gridBuy = 'CEMS_Real_Grid_Purchase(NTD)' # O^cost
        self.__output_drf = 'CEMS_DR_Feedback_Price(NTD)' # O_dr^feedback
        self.__output_CEMS_final = 'CEMS_Final_Price(NTD)' # O_comm^total
        self.__output_CEMS_eff = 'CEMS_Final_Price(NTD)' # \eta^cost
        # EM & EV
        self.__output_EMEV_chargePower = 'EMEV_charge'
        self.__output_EMEV_dischargePower = 'EMEV_discharge'
        self.__output_EMAVG = 'EM_AVG_SOC'
        self.__output_EMMIN = 'EM_MIN_SOC'
        self.__output_EM_totalPower = 'EM_totalPower'
        self.__output_EM_chargePower = 'EM_chargePower'
        self.__output_EM_dischargePower = 'EM_dischargePower'
        self.__output_EVAVG = 'EV_AVG_SOC'
        self.__output_EVMIN = 'EV_MIN_SOC'
        self.__output_EV_totalPower = 'EV_totalPower'
        self.__output_EV_chargePower = 'EV_chargePower'
        self.__output_EV_dischargePower = 'EV_dischargePower'
        # output
        self.arr_BP = []
        self.arr_rightEff = []
        self.arr_eff = []

        # csv file
        self.__BP = 'BaseParameter.csv'
        self.__EMresult = 'EM_user_result.csv'
        self.__EMnumber = 'EM_user_number.csv'
        self.__EVresult = 'EV_user_result.csv'
        self.__EVnumber = 'EV_user_number.csv'
        self.__TL = 'totalLoad_model.csv'
        self.__LHEMS_cost = 'LHEMS_cost.csv'
        self.__Cost = 'cost.csv'
        # target parm
        self.parm_PL = 'publicLoad'
        self.parm_PLSpend = 'publicLoadSpend(threeLevelPrice)'
        self.parm_Load = 'LoadSpend(threeLevelPrice)'
        self.parm_realGP = 'realGridPurchase'
        self.parm_DRF = 'demandResponse_feedbackPrice'
        self.__parm_EM = 'ElectricMotor'
        self.__parm_EV = 'ElectricVehicle'

        # flag
        self.__flag_ev = bool()
        self.__flag_em = bool()

    def combine_path(self, sequence_list):
        self.final_path = os.path.join(self.__str_fixPath, '/'.join([str(value) for value in sequence_list]))
        return os.path.exists(self.final_path)

    def getBP_realGP_DRF(self):
        df = pd.read_csv(os.path.join(self.final_path, self.__BP), header=None)
        real_grid_purchase = df[(df[1] == self.parm_realGP)].iloc[:,-1].values[0]
        dr_feedback_price = df[(df[1] == self.parm_DRF)].iloc[:,-1].values[0]
        load_spend_price = df[(df[1] == self.parm_Load)].iloc[:,-1].values[0]
        publicLoad = df[(df[1] == self.parm_PL)].iloc[:,-1].values[0]
        publicLoadSpend = df[(df[1] == self.parm_PLSpend)].iloc[:,-1].values[0]
        self.__flag_em = bool(int(df[(df[1] == self.__parm_EM)].iloc[:,-1].values[0]))
        self.__flag_ev = bool(int(df[(df[1] == self.__parm_EV)].iloc[:,-1].values[0]))
        return [real_grid_purchase, dr_feedback_price, load_spend_price, publicLoad, publicLoadSpend]

    def getEMEV_departureSOC(self): 
        if self.__flag_em: 
            df = pd.read_csv(os.path.join(self.final_path, self.__EMresult), header=None)
            avg_EM = f"{round(float(df.iloc[:,-4].mean())*100, 2)}%"
            min_EM = f"{round(float(df.iloc[:,-4].min())*100, 2)}%"
            df = pd.read_csv(os.path.join(self.final_path, self.__EMnumber), header=None)
            P_charge_EM = round(float(df.iloc[43:61, -2].sum()), 2)
            P_discharge_EM = round(float(df.iloc[43:61, -1].sum()), 2)
        else: 
            avg_EM = ''
            min_EM = ''
            P_charge_EM = ''
            P_discharge_EM = ''

        if self.__flag_ev: 
            df = pd.read_csv(os.path.join(self.final_path, self.__EVresult), header=None)
            avg_EV = f"{round(float(df.iloc[:,-4].mean())*100, 2)}%"
            min_EV = f"{round(float(df.iloc[:,-4].min())*100, 2)}%"
            df = pd.read_csv(os.path.join(self.final_path, self.__EVnumber), header=None)
            P_charge_EV = round(float(df.iloc[43:61, -2].sum()), 2)
            P_discharge_EV = round(float(df.iloc[43:61, -1].sum()), 2)
        else: 
            avg_EV = ''
            min_EV = ''
            P_charge_EV = ''
            P_discharge_EV = ''

        if ((P_charge_EM == '') or (P_discharge_EM == '')):
            charge_EMEV = P_charge_EV
            discharge_EMEV = P_discharge_EV
        elif ((P_charge_EV == '') or (P_discharge_EV == '')):
            charge_EMEV = P_charge_EM
            discharge_EMEV = P_discharge_EM
        else:
            charge_EMEV = P_charge_EM + P_charge_EV
            discharge_EMEV = P_discharge_EM + P_discharge_EV

        dict_EMEV_SOC = {
            'EM': {
                'avg': avg_EM,
                'min': min_EM,
                'P_total': P_charge_EM + P_discharge_EM,
                'P_charge': P_charge_EM,
                'P_discharge': P_discharge_EM,
            },
            'EV': {
                'avg': avg_EV,
                'min': min_EV,
                'P_total': P_charge_EV + P_discharge_EV,
                'P_charge': P_charge_EV,
                'P_discharge': P_discharge_EV,
            },
            'EMEV': {
                'P_charge': charge_EMEV,
                'P_discharge': discharge_EMEV,
            }
        }
        return dict_EMEV_SOC

    def getTL_MAX(self):
        df = pd.read_csv(os.path.join(self.final_path, self.__TL), header=None)
        return df.iloc[:,-2].max()

    def appendToArray(self, arr_bp, dict_SOC, tlmax, sequence_list): 
        if (arr_bp is None or dict_SOC is None or tlmax is None):
            self.arr_BP.append([self.final_path, sequence_list[-4], sequence_list[-3], sequence_list[-2], sequence_list[-1], '', '', '', '', '', '' '', '', '', ''])
        else:
            rgp = float(arr_bp[0])
            drf = float(arr_bp[1])
            lsp = float(arr_bp[2])
            pl = float(arr_bp[3])
            pls = float(arr_bp[4])
            self.arr_BP.append([self.final_path, 
            sequence_list[-4], 
            sequence_list[-3], 
            sequence_list[-2], 
            sequence_list[-1], 
            round(float(tlmax), 2), 
            round(float(pl), 2), 
            round(float(pls), 2), 
            round(lsp, 2), 
            round(rgp, 2), 
            round(drf, 2), 
            round(rgp-drf, 2), 
            f"{round((lsp-(rgp-drf))/lsp*100, 2)}%", 
            dict_SOC['EMEV']['P_charge'], 
            dict_SOC['EMEV']['P_discharge'], 
            dict_SOC['EM']['avg'], 
            # dict_SOC['EM']['min'], 
            dict_SOC['EV']['avg'], 
            # dict_SOC['EV']['min'], 
            # dict_SOC['EM']['P_total'], 
            dict_SOC['EM']['P_charge'], 
            dict_SOC['EM']['P_discharge'], 
            # dict_SOC['EV']['P_total'], 
            dict_SOC['EV']['P_charge'], 
            dict_SOC['EV']['P_discharge']])
    
    def showBP_df(self):
        return pd.DataFrame(self.arr_BP, columns=(self.__output_path, 
        self.__output_price, 
        self.__output_coml, 
        self.__output_weather, 
        self.__output_drmode, 
        self.__output_maxConsumption, 
        self.__output_publicLoad, 
        self.__output_publicLoadSpend, 
        self.__output_LoadSpend, 
        self.__output_gridBuy, 
        self.__output_drf, 
        self.__output_CEMS_final, 
        self.__output_CEMS_eff, 
        self.__output_EMEV_chargePower,
        self.__output_EMEV_dischargePower,
        self.__output_EMAVG, 
        # self.__output_EMMIN, 
        self.__output_EVAVG, 
        # self.__output_EVMIN, 
        # self.__output_EM_totalPower, 
        self.__output_EM_chargePower, 
        self.__output_EM_dischargePower, 
        # self.__output_EV_totalPower, 
        self.__output_EV_chargePower, 
        self.__output_EV_dischargePower))

    def getLHEMS_SaveingEffeiency(self):
        df = pd.read_csv(os.path.join(self.final_path, self.__LHEMS_cost), header=None)
        self.arr_eff.append(df.iloc[:,-2].tolist())
    
    def showLHEMS_df(self):
        return pd.DataFrame(self.arr_eff).rename(columns=lambda name: f"household{name+1}")
    
    def getLHEMS_Cost(self):
        df = pd.read_csv(os.path.join(self.final_path, self.__LHEMS_cost), header=None)
        df = df.sort_values(by=[97])
        lhems = df.iloc[:,1:97]
        final_pay_price = df.iloc[:,-3]
        df = pd.read_csv(os.path.join(self.final_path, self.__Cost), header=None)
        ghems_cost = df[(df[97] == 'total_load_price')].iloc[:,1:97]
        arr_tmp = []
        arr_sum_tmp=[]
        for i in range (len(lhems)):
            product = [x*y for x,y in zip(lhems.iloc[i,:].tolist(), ghems_cost.loc[3])]
            arr_tmp.append(product)
            arr_sum_tmp.append(pd.DataFrame(arr_tmp).loc[i].sum())
        
        for i in range(1,97):
            pd.DataFrame(arr_tmp).iloc[:,i-1] /= lhems[i].sum()
        
        arr_ratio = [(x-y)/x for x,y in zip(arr_sum_tmp, final_pay_price)]
        self.arr_rightEff.append(arr_ratio)

    def showLHEMS_RightEff(self):
        return pd.DataFrame(self.arr_rightEff).rename(columns=lambda name: f"household{name+1}")

if __name__ == "__main__":

    sp = StrPath()
    for single_list in product(*sp.loop_list):
        if(sp.combine_path(single_list)):
            arr_BPinfo = sp.getBP_realGP_DRF()
            total_load_max = sp.getTL_MAX()
            dict_EMEVSOC = sp.getEMEV_departureSOC()
            sp.appendToArray(arr_BPinfo, dict_EMEVSOC, total_load_max, single_list)
            # sp.getLHEMS_SaveingEffeiency()
            # sp.getLHEMS_Cost()

        else:
            sp.appendToArray(None, None, None, single_list)
    df_BP = sp.showBP_df()
    # df_LHEMS = sp.showLHEMS_df()
    # df_LHEMSright = sp.showLHEMS_RightEff()
    # print(df_BP, df_LHEMS, df_LHEMSright)

    # print(df_LHEMS.to_json(orient = 'records')) # covert to json
    df_BP.to_csv('df_BP.csv', index=False) # save to csv
    # df_LHEMS.to_csv('df_LHEMS.csv', index=False) # save to csv
    # df_LHEMSright.to_csv('df_LHEMSright.csv', index=False) # save to csv
    