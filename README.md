# fifty_DHEMS

> This project is about my final master drgree topic, the goal is optimization the 50 household's load comsuption deployment and community energy supplying.

> Use crontab setting execute time and shell script to do the script, the script will optimize 50 household load deployment first, then feedback the total amount of load to community, final community will optimized the energy supply we have

> `web.py` is automatically script for 
> 1. export related tables to csv into `download folder` 
> 2. screenshot web page into `customized folder`,
> 
> only use on windows OS, and related libs are in `anaconda ENV: webDriver`
---
### 2021/06/03

+ Commit Link [f521b6f](https://github.com/colin861209/fifty_DHEMS/commit/f521b6ffeb56e13600e8cb9f75031692987bd828)
+ **INITIAL** files from [DHEMS](https://github.com/colin861209/DHEMS) repository

---
### 2021/06/18

+ Commit Link [e5ea65a](https://github.com/colin861209/fifty_DHEMS/commit/e5ea65a307779ddf633804eefd2094d9c5e5b386)
+ Community Public Load
+ Demand Response `alpha constraint`
+ Type `bool` for flags

---
### 2021/06/25

+ Commit Link [4d08c46](https://github.com/colin861209/fifty_DHEMS/commit/4d08c4628223385aff8e2d4fdcc0d4acd275d0e9)
+ Community Public Load `decrease operation time` when demand response mode
+ Demand Response **remove** `alpha constraint`, it's not a good constraint
+ Demand Response **increase** `household participation`, household determine their time to participate.

---
### 2021/07/21

+ Commit Link [2cd9b4b](https://github.com/colin861209/fifty_DHEMS/commit/2cd9b4b2c37d3b0a5cbc1b0d2a744aa954a119e3)
+ GLPK `constraint` functionlization in GHEMS & LHEMS

---
### 2021/07/21

+ Commit Link [5f2bcb1](https://github.com/colin861209/fifty_DHEMS/commit/5f2bcb143db9db5ecb8709134763334e4165d7c9)
+ Community Public Load `force close` when demand response mode
+ Function SQL to insert & update status to DB

---
### 2021/09/11

+ Commit Link [f30a6a7](https://github.com/colin861209/fifty_DHEMS/commit/f30a6a73bfb23b6b177e3a9ca148b4376ed3f8d6)
+ Feature: Comfort level in LHEMS
