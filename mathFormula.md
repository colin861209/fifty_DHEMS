# DHEMS Mathematical formula

###### tags: `DHEMS`

## Community Enerage Management System (CEMS)

* $P^{j}_{grid}$ 第 j 個時刻之社區市電功率
* $\rho_{b}^{j}$ 第 j 個時刻之時間電價
* $\rho_{f}^{j}$ 第 j 個時刻之回饋報價 (定值)
* $\tau_{r}^{s}$ 即時備轉(Spinning Reserve)開啟時刻
* $\tau_{r}^{e}$ 即時備轉(Spinning Reserve)結束時刻
* Objective Function

$$
\min_{\substack {
    P_{grid}^{j}, j=0,...,J-1 \\ 
    \mu_{grid}^{j}, j=0,...,J-1 \\
    P_{sell}^{j}, j=0,...,J-1 \\ 
    P_{ESS}^{j}, j=0,...,J-1 \\
    P_{FC}^{j}, j=0,...,J-1 \\ 
    \mu_{FC}^{j}, j=0,...,J-1 \\
    P_{FC\_ON}^{j}, j=0,...,J-1 \\
    P_{FC\_OFF}^{j}, j=0,...,J-1 \\
    z_{i}^{j}, i=0,...,M-1,~j=0,...,J-1 \\
    \lambda_{i}^{j}, i=0,...,M-1,~j=0,...,J-1 \\
    r_{ca}^{j}, j=0,...,J-1,~ca \in A_{c1} \\
    r_{em,n}^{c,j}, n=1,...,N,j=0,...,J-1 \\
    r_{em,n}^{d,j}, n=1,...,N,j=0,...,J-1 \\
    \mu_{em,n}^{j}, n=1,...,N,j=0,...,J-1 \\
}}
\sum_{j=k}^{J-1} \rho_{b}^{j} (P^{j}_{grid} - P_{sell}^{j})T_{s} - \sum_{j=k}^{J-1}\sum_{n=1}^{N} \rho_{b}^{j}w_{n}^{j}P_{n}^{d,max}r_{em,n}^{d,j}T_{s} -
\sum_{j=\tau_{r}}^{\tau_{r}^{e}-1} \rho_{f}^{j} (P_{grid}^{avg}-P^{j}_{grid}) T_{s}
\quad
\begin{aligned}
  \tau_{r}=\tau_{r}^{s},& if \quad k<\tau_{r}^{s}\\
  \tau_{r}=k,& if \quad \tau_{r}^{s}\leq k<\tau_{r}^{e}\\
\end{aligned}
$$
 
* Deamnd Response
    * $d$ 抑低用電日
    * $P_{grid}^{j,d}$ 第d天第j時刻的市電功率
    * $P_{max}^{d}$ 第d天抑低用電時段市電最大功率
    * $P_{grid}^{avg}$ 抑低用電時段平均值 (基準用電容量 CBL)
    * $E_{s}$ 輔助服務時段最少降載度數
    * Formula
    
    $$ P_{max}^{d}= \max_{j=\tau_{r}^{s},...,\tau_{r}^{e}}P_{grid}^{j,d}, \quad d=1,...,5 $$
    $$ P_{grid}^{avg} = \frac{\sum_{d=1}^{D} P_{max}^{d}} {D} $$
  
    * Constraint
    
    $$ E_{s} \leq \sum_{j=\tau_{r}^{s}}^{\tau_{r}^{e}-1} (P_{grid}^{avg} - P_{grid}^{j})T_{s} $$
  
* Blanced Function
    
    $$ P^{j}_{grid} + P^{j}_{PV} + P^{j}_{FC} - P^{j}_{sell} - P^{j}_{ESS} = \sum_{n=1}^{N}(r_{em, n}^{c, j}P_{n}^{c,max}-r_{em, n}^{d, j}P_{n}^{d,max}) + \sum_{ca \in A_{c1}} P_{ca}^{j} + \sum_{u=1}^{U}(\sum_{a \in  A_{c1} \cup A_{c2} \cup A_{c3}} P_{u,a}^{j} + \sum_{a \in A_{uc}} P_{u, uc}^{j}) $$

* Grid & Sell
  <!-- * $P_{u, grid}^{j}$ 第 u 個住戶在第 j 個時刻消耗的市電功率 -->
  <!-- * $P_{u, grid}^{max}$ 第 u 個住戶的市電最大功率限制 -->
  <!-- * $\alpha_{u}^{j}$ 第 u 個住戶在第 j 個時刻同意使用多少百分比之市電功率 -->
    * $P_{grid}^{j, max}$ 社區第j時刻的市電最大功率限制
    * Formula
    
    $$ P_{grid}^{j,max} = \sum_{u=1}^{U} \alpha_{u}^{j} P_{u, grid}^{j,max} $$

    * Variable
    
    $$ 0 \leq P_{grid}^{j} \leq P_{grid}^{j,max} $$

    * Constraint
    
    $$ P_{grid}^{j} \leq \mu_{grid}^{j}P_{grid}^{j,max} $$

    $$ P_{sell}^{j} \leq [1 - \mu_{grid}^{j}] P_{sell}^{max} $$
    
    $$ P_{sell}^{j} \leq P_{FC}^{j} + P_{PV}^{j} $$

    <!-- * For GHEMS => LHEMS
    $$ P_{grid}^{max} = \sum_{u=1}^{U} \alpha_{u}^{j} P_{u, grid}^{max} $$
    $$ P_{grid}^{j} = \sum_{u=1}^{U} \alpha_{u}^{j} P_{u, grid}^{j} $$
    $$ 0 \leq P_{u,grid}^{j} \leq P_{u, grid}^{max} $$
    $$ 0 \leq \alpha_{u}^{j} \leq 1 $$ -->

* Battery
  
    <!-- * $P_{u,ESS}^{j}$ 第 u 個住戶在第 j 個時刻使用的電池功率 -->
    <!-- * $\beta_{u}^{j}$ 第 u 個住戶在第 j 個時刻同意使用多少百分比之電池功率 -->
    * Variable
  
    $$P_{discharge}^{max} \leq P^{j}_{ESS} \leq P_{charge}^{max}$$
    
    $$SOC^{min} \leq SOC_{j} \leq SOC^{max}$$
    
    * Constraint
  
    $$SOC_{j-1} + \sum_{j=k}^{J-1} \frac{P^{j}_{ESS}T_{s}}{C_{ESS}V_{ESS}} \geq SOC^{threshold}$$

    $$SOC_{j} = SOC_{j-1} + \frac {P^{j}_{ESS}T_{s}}{C_{ESS}V_{ESS}}$$
    
  ---
  * For constraint SOC need discharge 80% a day
    * Variable
  
    $$ \frac {P^{max}_{discharge}T_{s}}{C_{ESS}V_{ESS}} \leq SOC_{j}^{change} \leq \frac {P^{max}_{charge}T_{s}}{C_{ESS}V_{ESS}}$$

    $$ 0 \leq SOC_{j}^{+} \leq \frac {P^{max}_{charge}T_{s}}{C_{ESS}V_{ESS}}$$

    $$ 0 \leq SOC_{j}^{-} \leq \frac {P^{max}_{discharge}T_{s}}{C_{ESS}V_{ESS}}$$
        
    * Constraint
    $$ SOC_{j}^{change} = SOC_{j}^{+} - SOC_{j}^{-} $$

    $$ SOC_{j}^{change} = \frac {P^{j}_{ESS}T_{s}}{C_{ESS}V_{ESS}} $$

    $$ SOC_{j}^{change} =
      \left\{ 
        \begin{array}
          rSOC_{j}^{+}, \qquad P_{ESS}^{j} > 0\\
          SOC_{j}^{-}, \qquad P_{ESS}^{j} < 0
        \end{array}
      \right.
      $$

    $$ SOC_{j}^{+} \leq Z^{'} \frac{P_{charge}^{max}T_{s}}{C_{ESS}V_{ESS}}$$

    $$ SOC_{j}^{-} \leq (1-Z^{'}) \frac{P_{discharge}^{max}T_{s}}{C_{ESS}V_{ESS}}$$

    $$ \sum_{j=0}^{J-1} SOC_{j}^{-} \geq 0.8$$
  ---

* Community Public Load
    * Formula
    $$ P_{ca}^{j} =
      \left\{ 
        \begin{array}{c}
          r_{ca}^{j} P_{ca}^{max}, \qquad &\forall k \in [\tau_{ca}^{s}, \tau_{ca}^{e}]\\
          0 \qquad, \qquad &otherwise
        \end{array}
      \right.
    $$
    * Variable
      $$ r_{ca}^{j} \in \{0,1\}, \qquad \forall j \in [\tau_{ca}^{s}, \tau_{ca}^{e}] $$

      $$ r_{ca}^{j} = 0, \qquad \forall j \in [0,J-1] \backslash [\tau_{ca}^{s}, \tau_{ca}^{e}] $$
      
      $$ \forall ca \in A_{c1} $$
    * Constraint
      $$ \sum_{k=0}^{J-1} r_{ca}^{j} \geq Q_{ca} - |d|, \qquad \forall d \in [\tau_{ca}^{s}, \tau_{ca}^{e}] \cap [\tau_{r}^{s}, \tau_{r}^{e}] $$
  ---
* Electric Motor
    * $r_{em, n}^{c, j}$ 第n充電柱在第j時刻的可充電旗標
    * $r_{em, n}^{d, j}$ 第n充電柱在第j時刻的可放電旗標
    * $SOC_{em}^{max} SOC_{em}^{min}$ 電動機車SOC最大最小值
    * $SOC_{em}^{threshold}$ 電動機車離場時規定SOC值
    * $\tau_{em, n}^{s}$ 第n充電柱進場電動機車時刻
    * $\tau_{em, n}^{e}$ 第n充電柱離場電動機車時刻
    * Formula
      $$ w_{n}^{j}=\frac{E_{n}^{cap}(SOC_{em,n}^{max}-SOC_{em,n}^{j})}{P_{n}^{max}(\tau_{em,n}^{e}-j)} $$
    * Variable
      $$ r_{em, n}^{c, j} \in \{0,1\}, \qquad \forall j \in [\tau_{em, n}^{s}, \tau_{em, n}^{e}], \forall n \in [0,N] $$

      $$ r_{em, n}^{c, j} = 0, \qquad \forall j \in [0,J-1] \backslash [\tau_{em, n}^{s}, \tau_{em, n}^{e}], \forall n \in [0,N] $$

      $$ r_{em, n}^{d, j} \in \{0,1\}, \qquad \forall j \in [\tau_{em, n}^{s}, \tau_{em, n}^{e}], \forall n \in [0,N] $$

      $$ r_{em, n}^{d, j} = 0, \qquad \forall j \in [0,J-1] \backslash [\tau_{em, n}^{s}, \tau_{em, n}^{e}], \forall n \in [0,N] $$
      <!-- $$ SOC_{em}^{min} \leq SOC_{em, n}^{j} \leq SOC_{em}^{max} $$ -->

    * Constraint
      $$ r_{em,n}^{c,j} \leq \mu_{em, n}^{j}, \qquad \forall n \in [0,N] $$

      $$ r_{em,n}^{d,j} \leq (1-\mu_{em, n}^{j}), \qquad \forall n \in [0,N] $$

      $$ SOC_{em}^{min} \leq SOC_{em, n}^{j-1} + (\frac{P_{n}^{c, max}r_{em, n}^{c,j}T_{s}}{E_{n}^{cap}} - \frac{P_{n}^{d, max}r_{em, n}^{d, j}T_{s}}{E_{n}^{cap}}), \qquad \forall n \in [0,N] $$
      
      $$ SOC_{em}^{threshold} \leq SOC_{em, n}^{j-1} + \sum_{j=\tau_{em, n}^{s}}^{\tau_{em, n}^{e}-1} (\frac{P_{n}^{c, max}r_{em, n}^{c,j}T_{s}}{E_{n}^{cap}} - \frac{P_{n}^{d, max}r_{em, n}^{d, j}T_{s}}{E_{n}^{cap}}), \qquad \forall n \in [0,N] $$

    <!-- * For GHEMS => LHEMS
    $$ P_{ESS}^{j} = \sum_{u=1}^{U} \beta_{u}^{j} P_{u,ESS}^{j} $$
    $$ P_{u,discharge}^{max} \leq P_{u,ESS}^{j} \leq P_{u,charge}^{max} $$
    $$ 0 \leq \beta_{u}^{j} \leq 1 $$ -->

  <!-- * FC -->

    <!-- $$ 0 \leq P^{j}_{FC} \leq P_{FC\_max}$$ -->

    <!-- $$ P^{j}_{FC\_ON} + P^{j}_{FC\_OFF} = P^{j}_{FC}$$ -->

    <!-- $$ P^{j}_{FC\_ON} \leq \mu^{j}_{FC}P_{FC\_max}$$ -->

    <!-- $$ \mu^{j}_{FC}P_{FC\_min} \leq P^{j}_{FC\_ON}$$ -->

    <!-- $$ P^{j}_{FC\_OFF} \leq [1 - \mu^{j}_{FC}]P_{FC\_OFF}$$ -->

    <!-- $$ 0 \leq \lambda^{j}_{i} \leq z^{j}_{i} \quad i=0,1,...,m-1$$ -->

    <!-- $$ \**sum_**{i=0}^{m-1} z^{j}_{i} =1$$ -->

    <!-- $$ \begin{aligned}
    P^{j}_{FC} &= 0 \cdot z^{j}_{0} \\
    &+ 0.35 \cdot z^{j}_{1} + (1.545-0.35) \cdot \lambda^{j}_{1} \\
    &+ 1.545 \cdot z^{j}_{2} + (2.740-1.545) \cdot \lambda^{j}_{2} \\
    &+ 3.935 \cdot z^{j}_{3} + (3.935-2.740) \cdot \lambda^{j}_{3} \\
    &+ 5.130 \cdot z^{j}_{4} + (5.130-3.935) \cdot \lambda^{j}_{4} \\
    \end{aligned} $$ -->

    <!-- $$ \begin{aligned}
    P^{j}_{FCT} &= 0 \cdot z^{j}_{0} \\
    &+ 0.6283 \cdot z^{j}_{1} + (3.0617-0.6283) \cdot \lambda^{j}_{1} \\
    &+ 3.0617 \cdot z^{j}_{2} + (5.8717-3.0617) \cdot \lambda^{j}_{2} \\
    &+ 5.8717 \cdot z^{j}_{3} + (9.0692-8.8717) \cdot \lambda^{j}_{3} \\
    &+ 9.0692 \cdot z^{j}_{4} + (12.8214-9.0692) \cdot \lambda^{j}_{4} \\
    \end{aligned} $$ -->

## Home Enerage Management System (HEMS)
* $u$ 代表第u個家庭
* $a$ 代表第a個設備負載
* $j$ 代表第j個時刻
* $P^{j}_{u,grid}$ 第 u 個住戶在第 j 個時刻消耗的市電功率

$$ \min_{\substack{
    r_{u,a}^{j},~j=0,...,J-1,~a \in A_{u, c1} \cup A_{u, c2} \cup A_{u, c3}\\ 
    \delta_{u,a}^{j},~j=0,...,J-1,~a \in A_{u, c2} \cup A_{u, c3}\\ 
    P_{u, a}^{j},~j=0,...,J-1,~a \in A_{u, c3}\\
    P_{u, grid}^{j},~j=0,...,J-1\\
    \alpha_{u}^{j}, j=0,..., J-1
}}
\sum_{j=k}^{J-1} \rho_{b}^{j}P^{j}_{u,grid} T_{s} +
\sum_{j=\tau_{r}}^{\tau_{r}^{e}-1} D_{u}^{j}\rho_{f}^{j}P^{j}_{u,grid} T_{s},
\quad 
\begin{aligned}
  \tau_{r}=\tau_{r}^{s},& if \quad k<\tau_{r}^{s}\\
  \tau_{r}=k,&if \quad \tau_{r}^{s}\leq k<\tau_{r}^{e}\\ 
\end{aligned}
$$

<!-- HEMS Constraint -->
* Demand response
    
	* $D_{u}^{j}$: 住戶u在j時刻是否參與輔助服務
	* $\alpha_{u}^{j}$: 住戶 u 在 j 時刻最大市電縮小百分比
    * Formula

    $$ D_{u}^{j} \in \{0, 1\} $$

    <!-- $$ P_{s}=E_{s}/T_{s} $$
    
    $$ P_{s}^{j}=\frac{P_{s}}{\tau_{r}^{e}-\tau_{r}^{s}-1}, \qquad \forall j \in [\tau_{r}^{s}, \tau_{r}^{e}] $$
    
    $$ P_{s}^{j}=\sum_{u=1}^{U}\frac{D_{u}^{j}}{\sum_{j=\tau_{r}^{s}}^{\tau_{r}^{e}-1}D_{u}^{j}}P_{s}, \qquad \forall j \in [\tau_{r}^{s}, \tau_{r}^{e}] $$ -->
    
    * Variable
    
    $$ \alpha_{u}^{j} \in [0, 1] $$
    
    <!-- $$ \omega_{u}^{j} = \frac{D_{u}^{j}P_{u, grid}^{j}}{\sum_{u=1}^{U} D_{u}^{j}P_{u, grid}^{j}}  $$
    
    $$ \omega_{u}^{j} =
    \left\{ 
      \begin{array}{c}
        \frac{P_{u, grid}^{j}}{\sum_{u=1}^{U} D_{u}^{j}P_{u, grid}^{j}}, &\qquad D_{u}^{j} = 1\\
        0 \qquad,&\qquad D_{u}^{j} = 0
      \end{array}
    \right.
    $$ -->

    * constraint
    
    $$ (1-D_{u}^{j}) \leq \alpha_{u}^{j} \leq 1, \qquad \forall j \in [\tau_{r}^{s}, \tau_{r}^{e}] $$
    
    $$ \alpha_{u}^{j} = 1, \qquad  \forall j \in [0, J-1] \backslash  [\tau_{r}^{s}, \tau_{r}^{e}] $$

* Balanced Function
	* $A_{u,uc}$第u個住戶的不可控負載類型
	* $P_{u,uc}^{j}$第u個住戶在第j個時刻的不可控負載功率

    $$ P_{u,grid}^{j} - P_{u,ESS}^{j}= \sum_{a \in A_{u, c1} \cup A_{u, c2} \cup A_{u, c3}} P_{u,a}^{j}  + \sum_{a \in A_{u, uc}} P_{u,uc}^{j}$$

* Pgrid
    * Variable 
    
    $$ 0 \leq P_{u, grid}^{j} \leq P_{u, grid}^{max} $$
    *  Constraint
  
    $$ 0 \leq P_{u,grid}^{j} \leq \alpha_{u}^{j}P_{u, grid}^{max} $$

<!-- * Battery
  
  $$ P_{u,charge}^{max} = \frac{P_{charge}^{max}}{U}$$
  
  $$ P_{u,discharge}^{max} = \frac{P_{discharge}^{max}}{U}$$
  
  $$ V_{u,ESS} = \frac{V_{ESS}}{U}$$
  
  $$ P_{u,discharge}^{max} \leq P_{u,ESS}^{j} \leq P_{u,charge}^{max} $$

  $$ SOC^{min} \leq SOC_{j-1} + \frac {P^{j}_{u,ESS}T_{s}}{C_{ESS}V_{u,ESS}} \leq SOC^{max} $$

  $$ SOC_{j-1} + \sum_{j=k}^{J-1} \frac{P^{j}_{u,ESS}T_{s}}{C_{ESS}V_{u,ESS}} \geq SOC^{threshold} $$

  $$ SOC_{j} = SOC_{j-1} + \frac {P^{j}_{u,ESS}T_{s}}{C_{ESS}V_{u,ESS}} $$ -->

* Loads Constraint
  <!-- Uncontrollable load Constraint -->
  * Uncontrollable load
    $$ P_{u,uc}^{j} =
    \left\{ 
      \begin{array}
        rr_{u,uc}^{j} P_{u,uc}^{}, \qquad &\forall k \in [\tau_{u}^{s}, \tau_{u}^{e}]\\
        0 \qquad, \qquad &otherwise
      \end{array}
    \right.
    $$

    $$ r_{u,uc}^{j} \in \{0,1\}, \qquad \forall j \in [\tau_{u}^{s}, \tau_{u}^{e}] $$
    $$ P_{u,uc}^{} \in [0.3, 1.0] $$
      
  <!-- Deploeyment load Constraint -->
  * Deploeyment load
    $$ P_{u,a}^{j} =
    \left\{ 
      \begin{array}
        rr_{u,a}^{j} P_{u,a}^{max}, \qquad &\forall k \in [\tau_{u,a}^{s}, \tau_{u,a}^{e}]\\
        0 \qquad, \qquad &otherwise
      \end{array}
    \right.
    $$

    $$ r_{u,a}^{j} \in \{0,1\}, \qquad \forall j \in [\tau_{u,a}^{s}, \tau_{u,a}^{e}] $$

    $$ r_{u,a}^{j} = 0, \qquad \forall j \in [0,J-1] \backslash [\tau_{u,a}^{s}, \tau_{u,a}^{e}] $$

    $$ \forall a \in A_{u,c1} $$

    $$ \sum_{k=0}^{J-1} r_{u,a}^{j} \geq Q_{a} $$

    $$ \forall a \in A_{u,c2} \cup A_{u,c3} $$

    $$ \sum_{k= \tau_{u,a}^{s}}^{\tau_{u,a}^{e}- \Gamma_{u, a} - 1} \delta_{u,a}^{j} = 1 $$

    $$ r_{u,a}^{j+n} \geq \delta_{u,a}^{j}, \qquad n = 0,...,\Gamma_a-1 $$

    $$ \forall a \in A_{u,c3} $$

    $$ \psi_{u, a}^{j+n} \geq \delta_{u,a}^{j} \sigma_{u,a}^{n} $$

## CEMS & HEMS Cost

##### HEMS
* $T_{u}^{price}$ 各住戶原始購買市電電費
  $$ T_{u}^{price} = \sum_{j=0}^{J-1}\rho^{j}P_{u,grid}^{j}T_{s} $$

* $O_{u}^{ca, cost}$ 各住戶公設花費
  $$ O_{u}^{ca, cost} = \frac{O_{ca}^{cost}}{U} $$

* $T_{u}^{total, price}$ 各住戶原始總花費
  $$ T_{u}^{total, price} = T_{u}^{price}+O_{u}^{ca, cost} $$

* $O_{u}^{cost}$ 各住戶最佳化市電花費
  $$ O_{u}^{cost} = \frac{T_{u}^{total, price}}{\sum_{u=1}^{U}T_{u}^{total, price}} \times O_{total}^{cost} $$

* $O_{u}^{dr, feedback}$ 各住戶輔助服務回饋
  $$ O_{u}^{dr, feedback} = \frac{\sum_{j=\tau_{r}^{s}}^{\tau_{r}^{e}-1} D_{u}^{j}}{\sum_{u=1}^{U}\sum_{j=\tau_{r}^{s}}^{\tau_{r}^{e}-1}D_{u}^{j}} \times O_{dr}^{feedback} $$

* $O_{u}^{total, cost}$ 各住戶最佳化總花費
  $$ O_{u}^{total, cost} = O_{u}^{cost} - O_{u}^{dr, feedback} $$

* $\eta_{u}^{cost}$ 各住戶節省電費比
  $$ \eta_{u}^{cost} = \frac{T_{u}^{total, price}-O_{u}^{total, cost}}{T_{u}^{total, price}} $$

##### CEMS
* $O_{ca}^{cost}$ 社區公設花費
  $$ O_{ca}^{cost} = \sum_{j=0}^{J-1}\rho^{j}P_{ca}^{j}T_{s} $$

* $O_{total}^{cost}$ 社區總市電花費
  $$ O_{total}^{cost} = \sum_{j=0}^{J-1}\rho^{j}P_{grid}^{j}T_{s} $$

* $O_{dr}^{feedback}$ 社區輔助服務回饋
  $$ O_{dr}^{feedback} = \sum_{j=\tau_{r}^{s}}^{\tau_{r}^{e}-1} \rho_{f}^{j} (P_{grid}^{avg}-P^{j}_{grid}) T_{s}$$