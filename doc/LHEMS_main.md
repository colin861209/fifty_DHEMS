# src/LHEMS.cpp ~ LHEMS10.cpp

Each file process `5` households simulation in shell script `main_fifty.sh`

## Program Main Flow:
1. Get DB table `BaseParameter` values
2. Determine `BaseParameter` parameter **real time** is `0` or `1`, trancate related DB tables and do new simulation when `0`, continue simulation when `1`
3. Get HEMS flag values
4. Get DB table `load list select` loads category's amount, then get each loads information from DB table `load list`
5. Get demand response information
6. Denfine variable name and record its length which will use in GLPK later
7. Get electric price data
8. Random uncontrollable load data or use old one
9. Do optmize and saving the result to related DB tables
10. Calculate **controllable loads consumption** then update to DB table `totalLoad_model`
11. Calculate HEMS cost
12. Update simulate time block if finish 5 households simulation
13. Update household number to do simulation
14. Close `HEMS uncontrollable load generate flag` after finish simulation