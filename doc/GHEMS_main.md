# src/GHEMS.cpp

## Program Main Flow:
1. Get DB table `BaseParameter` values
2. Get demand response information
3. Get CEMS flag values
4. Get DB table `EM_Parameter` values
5. Get DB table `EV_Parameter` values
6. Determine `BaseParameter` parameter **real time** is `0` or `1`, trancate related DB tables and do new simulation when `0`, continue simulation when `1`
7. Insert new EMEV users into parking lot
8. Random uncontrollable load data or use old one
9. Denfine variable name and record its length which will use in GLPK later
10. Get electric price data
11. Get weather data
12. Get HEMS loads consumption from DB table `totalLoad_model`
13. Insert CEMS power supply information
14. Setting grid power array with demand response `Power baseline (CBL)`
15. Do optmize and saving the result to related DB tables
16. Update EMEV users SOC after charging or discharging
17. Calculate CEMS cost
18. Calculate each HEMS cost
19. Update ESS SOC value
20. Update simulate time block
21. Close `EM & EV & CEMS uncontrollable load generate flag` after finish simulation