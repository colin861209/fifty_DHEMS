
import pandas as pd
import os
from itertools import product
import numpy as np

class StrPath:
    
    def __init__(self) -> None:
        self.__str_fixPath = r"C:\Users\sonu\Desktop\howThesis\HEMSresult"
        self.__dir_mainFolder = ['13.EMEV_final'] # main folder
        self.__dir_price = ['summer_price', 'comed_price', ] # price
        self.__dir_comlv = ['EM_EV_coml_HEMSuc', 'EM_EV_noComl_HEMSuc', ] # comfort level
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
        self.__output_LoadSpend = 'CEMS_Load_Spend(NTD)'
        self.__output_gridBuy = 'CEMS_Real_Grid_Purchase(NTD)'
        self.__output_drf = 'CEMS_DR_Feedback_Price(NTD)'
        self.__output_CEMS_final = 'CEMS_Final_Price(NTD)'
        # output
        self.arr_BP = []
        self.arr_rightEff = []
        self.arr_eff = []

        # csv file
        self.__BP = 'BaseParameter.csv'
        self.__TL = 'totalLoad_model.csv'
        self.__LHEMS_cost = 'LHEMS_cost.csv'
        self.__Cost = 'cost.csv'
        # target parm
        self.parm_Load = 'LoadSpend(threeLevelPrice)'
        self.parm_realGP = 'realGridPurchase'
        self.parm_DRF = 'demandResponse_feedbackPrice'

    def combine_path(self, sequence_list):
        self.final_path = os.path.join(self.__str_fixPath, '/'.join([str(value) for value in sequence_list]))
        return os.path.exists(self.final_path)

    def getBP_realGP_DRF(self):
        df = pd.read_csv(os.path.join(self.final_path, self.__BP), header=None)
        real_grid_purchase = df[(df[1] == self.parm_realGP)].iloc[:,-1].values[0]
        dr_feedback_price = df[(df[1] == self.parm_DRF)].iloc[:,-1].values[0]
        load_spend_price = df[(df[1] == self.parm_Load)].iloc[:,-1].values[0]
        return real_grid_purchase, dr_feedback_price, load_spend_price

    def getTL_MAX(self):
        df = pd.read_csv(os.path.join(self.final_path, self.__TL), header=None)
        return df.iloc[:,-2].max()

    def appendToArray(self, rgp, drf, lsp, tlmax, sequence_list): 
        self.arr_BP.append([self.final_path, sequence_list[-4], sequence_list[-3], sequence_list[-2], sequence_list[-1], tlmax, lsp, rgp, drf, (float(rgp)-float(drf))])
    
    def showBP_df(self):
        return pd.DataFrame(self.arr_BP, columns=(self.__output_path, self.__output_price, self.__output_coml, self.__output_weather, self.__output_drmode, self.__output_maxConsumption, self.__output_LoadSpend, self.__output_gridBuy, self.__output_drf, self.__output_CEMS_final))

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
            real_grid_purchase, dr_feedback, load_spend_price = sp.getBP_realGP_DRF()
            total_load_max = sp.getTL_MAX()
            sp.getLHEMS_SaveingEffeiency()
            sp.appendToArray(real_grid_purchase, dr_feedback, load_spend_price, total_load_max, single_list)
            sp.getLHEMS_Cost()

        else:
            sp.appendToArray('', '', '', single_list)
    
    df_BP = sp.showBP_df()
    df_LHEMS = sp.showLHEMS_df()
    df_LHEMSright = sp.showLHEMS_RightEff()
    # print(df_BP, df_LHEMS, df_LHEMSright)

    # print(df_LHEMS.to_json(orient = 'records')) # covert to json
    df_BP.to_csv('df_BP.csv', index=False) # save to csv
    df_LHEMS.to_csv('df_LHEMS.csv', index=False) # save to csv
    df_LHEMSright.to_csv('df_LHEMSright.csv', index=False) # save to csv
    