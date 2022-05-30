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
    P_{grid}^{j}, ~j=0,...,J-1 \\ 
    P_{ESS}^{j}, ~j=0,...,J-1 \\
    r_{p}^{j}, ~j=0,...,J-1, ~p \in A_{s} \cup A_{d} \\
    r_{em,n}^{c,j}, ~m=1,...,M, ~j=0,...,J-1 \\
    r_{em,n}^{d,j}, ~m=1,...,M, ~j=0,...,J-1 \\
    \mu_{em,n}^{j}, ~m=1,...,M, ~j=0,...,J-1 \\
    r_{ev,n}^{c,j}, ~v=1,...,V, ~j=0,...,J-1 \\
    r_{ev,n}^{d,j}, ~v=1,...,V, ~j=0,...,J-1 \\
    \mu_{ev,n}^{j}, ~v=1,...,V, ~j=0,...,J-1 \\
}}
\sum_{j=k}^{J-1} \rho_{b}^{j} (P^{j}_{grid}-P_{total}^{d,j}) T_{s} - 
\sum_{j=\tau_{r}}^{\tau_{r}^{e}} \rho_{f}^{j} (\bar{P}_{base}-P^{j}_{grid}) T_{s},
\quad if \quad k < \tau_{r}^{s}
$$

$$
\min_{\substack {
    P_{grid}^{j}, j=0,...,J-1 \\ 
    P_{ESS}^{j}, j=0,...,J-1 \\
    r_{p}^{j}, j=0,...,J-1,~p \in A_{s} \cup A_{d} \\
    r_{em,n}^{c,j}, ~m=1,...,M, ~j=0,...,J-1 \\
    r_{em,n}^{d,j}, ~m=1,...,M, ~j=0,...,J-1 \\
    \mu_{em,n}^{j}, ~m=1,...,M, ~j=0,...,J-1 \\
    r_{ev,n}^{c,j}, ~v=1,...,V, ~j=0,...,J-1 \\
    r_{ev,n}^{d,j}, ~v=1,...,V, ~j=0,...,J-1 \\
    \mu_{ev,n}^{j}, ~v=1,...,V, ~j=0,...,J-1 \\
}}
\sum_{j=k}^{J-1} \rho_{b}^{j} (P^{j}_{grid}-P_{total}^{d,j}) T_{s} - 
\sum_{j=k}^{\tau_{r}^{e}} \rho_{f}^{j} (\bar{P}_{base}-P^{j}_{grid}) T_{s},
\quad if \quad \tau_{r}^{s} \leq k \leq \tau_{r}^{e}
$$

$$
\min_{\substack {
    P_{grid}^{j}, j=0,...,J-1 \\ 
    P_{ESS}^{j}, j=0,...,J-1 \\
    r_{p}^{j}, j=0,...,J-1,~p \in A_{s} \cup A_{d} \\
    r_{em,n}^{c,j}, ~m=1,...,M, ~j=0,...,J-1 \\
    r_{em,n}^{d,j}, ~m=1,...,M, ~j=0,...,J-1 \\
    \mu_{em,n}^{j}, ~m=1,...,M, ~j=0,...,J-1 \\
    r_{ev,n}^{c,j}, ~v=1,...,V, ~j=0,...,J-1 \\
    r_{ev,n}^{d,j}, ~v=1,...,V, ~j=0,...,J-1 \\
    \mu_{ev,n}^{j}, ~v=1,...,V, ~j=0,...,J-1 \\
}}
\sum_{j=k}^{J-1} \rho_{b}^{j} (P^{j}_{grid}-P_{total}^{d,j}) T_{s},
\quad if \quad k > \tau_{r}^{e}
$$


* EMEV Formula

$$
P_{total}^{d,j} = \sum_{m=1}^{M}P_{m}^{d, max}r_{em,m}^{d,j} + \sum_{v=1}^{V}P_{v}^{d, max}r_{ev,v}^{d,j}
$$

$$
P_{total}^{c,j} = \sum_{m=1}^{M}P_{m}^{c, max}r_{em,m}^{d,j} + \sum_{v=1}^{V}P_{v}^{c, max}r_{ev,v}^{d,j}
$$


* Deamnd Response
    * $n$ 抑低用電日
    * $\bar{P}_{base}$ 需量反應用電基準值 (基準用電容量 CBL)
    * $E_{s}$ 輔助服務時段最少降載度數

    * Formula
    $$ \bar{P}_{base} = \frac{1}{N}\frac{1}{(\tau_{r}^{e} - \tau_{r}^{s})}\sum_{n=1}^{N}\sum_{j=\tau_{r}^{s}}^{\tau_{r}^{e}} P_{-n}^{j} $$

    * Constraint
    $$ E_{s} \leq \sum_{j=\tau_{r}^{s}}^{\tau_{r}^{e}} (\bar{P}_{base} - P_{grid}^{j})T_{s} $$
  
    $$ P_{total}^{c,j} \leq P_{total}^{d,j} + P_{grid}^{j, max}-P_{discharge}^{j,max}+P_{pv}^{j}, \quad j=k...J-1 $$

* Blanced Function
    
    $$ P^{j}_{grid} + P^{j}_{PV} + P_{total}^{d,j} - P^{j}_{ESS} = P_{total}^{c,j} + \sum_{p \in A_{s} \cup A_{d}} P_{p}^{j} + \sum_{a \in A_{uc}}P_{uc}^{j} + \sum_{u=1}^{U}\sum_{a \in  A_{c1} \cup A_{c2} \cup A_{c3}} P_{u,a}^{j} $$

* Grid & Sell
    * $P_{grid}^{j, max}$ 社區第j時刻的市電最大功率限制
    * Variable
    
    $$ 0 \leq P_{grid}^{j} \leq
    \left\{ 
        \begin{array}{c}
          \bar{P}_{base}, &\quad \forall j \in [\tau_{r}^{s}, \tau_{r}^{e}]\\
          P_{grid}^{max}, & otherwise
        \end{array}
      \right.
    $$

* Battery
  
    * Variable
  
    $$P_{discharge}^{max} \leq P^{j}_{ESS} \leq P_{charge}^{max}$$
    
    $$SOC_{ESS}^{min} \leq SOC_{j} \leq SOC_{ESS}^{max}$$
    
    * Constraint
  
    $$SOC_{j-1} + \sum_{j=k}^{J-1} \frac{P^{j}_{ESS}T_{s}}{C_{ESS}V_{ESS}} \geq SOC_{ESS}^{threshold}$$

    $$SOC_{j} = SOC_{j-1} + \frac {P^{j}_{ESS}T_{s}}{C_{ESS}V_{ESS}}$$

* Community Public Load
    * Formula
    $$ P_{p}^{j} =
      \left\{ 
        \begin{array}{c}
          r_{p}^{j} P_{p}^{max}, \qquad &\forall j \in [\tau_{p}^{s}, \tau_{p}^{e}]\\
          0 \qquad, \qquad &otherwise
        \end{array}
      \right.
    $$
    * Variable
      $$ r_{p}^{j} \in \{0,1\}, \qquad \forall j \in [\tau_{p}^{s}, \tau_{p}^{e}] $$

      $$ r_{p}^{j} = 0, \qquad \forall j \in [0,J-1] \backslash [\tau_{p}^{s}, \tau_{p}^{e}] $$
      
      $$ \forall p \in A_{s} \cup A_{d} $$
    * Constraint
      * Stoppable
        * Formula
        $$ Q_{r} = |[\tau_{p}^{s}, \tau_{p}^{e}] \cap [\tau_{r}^{s}, \tau_{r}^{e}]| $$
        
        * Constraint
        $$ \sum_{j=0}^{J-1}r_{p}^{j} \geq (Q_{p}-Q_{r}),\quad \forall p \in A_{s} $$

      * Deferrable
        * Constraint
        $$ \sum_{j=0}^{J-1}r_{p}^{j} \geq Q_{p},\quad \forall p \in A_{d} $$

  ---
* Electric Motor
    * $r_{em, m}^{c, j}$ 第n充電柱在第j時刻的可充電旗標
    * $r_{em, m}^{d, j}$ 第n充電柱在第j時刻的可放電旗標
    * $SOC_{em}^{max} SOC_{em}^{min}$ 電動機車SOC最大最小值
    * $SOC_{em}^{threshold}$ 電動機車離場時規定SOC值
    * $\tau_{em, m}^{s}$ 第n充電柱進場電動機車時刻
    * $\tau_{em, m}^{e}$ 第n充電柱離場電動機車時刻
    * $\eta_{m}^{c}$ 第m充電柱充電效率
    * $\eta_{m}^{d}$ 第m充電柱放電效率

    * Variable
    
      $$ r_{em, m}^{c, j} \in \{0,1\}, \qquad \forall j \in [\tau_{em, m}^{s}, \tau_{em, m}^{e}], \forall m \in [0,M] $$

      $$ r_{em, m}^{c, j} = 0, \qquad \forall j \in [0,J-1] \backslash [\tau_{em, m}^{s}, \tau_{em, m}^{e}], \forall m \in [0,M] $$

      $$ r_{em, m}^{d, j} \in \{0,1\}, \qquad \forall j \in [\tau_{em, m}^{s}, \tau_{em, m}^{e}], \forall m \in [0,M] $$

      $$ r_{em, m}^{d, j} = 0, \qquad \forall j \in [0,J-1] \backslash [\tau_{em, m}^{s}, \tau_{em, m}^{e}], \forall m \in [0,M] $$

    * Constraint
      $$ r_{em,m}^{c,j} \leq \mu_{em, m}^{j}, \qquad \forall m \in [0,M] $$

      $$ r_{em,m}^{d,j} \leq (1-\mu_{em, m}^{j}), \qquad \forall m \in [0,M] $$

      $$ SOC_{em}^{min} \leq SOC_{em, m}^{j-1} + (\frac{\eta_{m}^{c}P_{m}^{c, max}r_{em, m}^{c,j}T_{s}}{E_{m}^{cap}} - \frac{\eta_{m}^{d}P_{m}^{d, max}r_{em, m}^{d, j}T_{s}}{E_{m}^{cap}}), \qquad \forall m \in [0,M] $$
      
      $$ SOC_{em}^{threshold} \leq SOC_{em, m}^{j-1} + \sum_{j=\tau_{em, m}^{s}}^{\tau_{em, m}^{e}} (\frac{\eta_{m}^{c}P_{m}^{c, max}r_{em, m}^{c,j}T_{s}}{E_{m}^{cap}} - \frac{\eta_{m}^{d}P_{m}^{d, max}r_{em, m}^{d, j}T_{s}}{E_{m}^{cap}}), \qquad \forall m \in [0,M] $$
  ---
* Electric Vehicle
    * $r_{ev, v}^{c, j}$ 第v充電柱在第j時刻的可充電旗標
    * $r_{ev, v}^{d, j}$ 第v充電柱在第j時刻的可放電旗標
    * $SOC_{ev}^{max} SOC_{ev}^{min}$ 電動機車SOC最大最小值
    * $SOC_{ev}^{threshold}$ 電動機車離場時規定SOC值
    * $\tau_{ev, v}^{s}$ 第v充電柱進場電動機車時刻
    * $\tau_{ev, v}^{e}$ 第v充電柱離場電動機車時刻
    * $\eta_{v}^{c}$ 第v充電柱充電效率
    * $\eta_{v}^{d}$ 第v充電柱放電效率
    * Variable
    
      $$ r_{ev, v}^{c, j} \in \{0,1\}, \qquad \forall j \in [\tau_{ev, v}^{s}, \tau_{ev, v}^{e}], \forall v \in [0,V] $$

      $$ r_{ev, v}^{c, j} = 0, \qquad \forall j \in [0,J-1] \backslash [\tau_{ev, v}^{s}, \tau_{ev, v}^{e}], \forall v \in [0,V] $$

      $$ r_{ev, v}^{d, j} \in \{0,1\}, \qquad \forall j \in [\tau_{ev, v}^{s}, \tau_{ev, v}^{e}], \forall v \in [0,V] $$

      $$ r_{ev, v}^{d, j} = 0, \qquad \forall j \in [0,J-1] \backslash [\tau_{ev, v}^{s}, \tau_{ev, v}^{e}], \forall v \in [0,V] $$

    * Constraint
      $$ r_{ev,m}^{c,j} \leq \mu_{ev, v}^{j}, \qquad \forall v \in [0,V] $$

      $$ r_{ev,m}^{d,j} \leq (1-\mu_{ev, v}^{j}), \qquad \forall v \in [0,V] $$

      $$ SOC_{ev}^{min} \leq SOC_{ev, v}^{j-1} + (\frac{\eta_{v}^{c}P_{v}^{c, max}r_{ev, v}^{c,j}T_{s}}{E_{v}^{cap}} - \frac{\eta_{v}^{d}P_{v}^{d, max}r_{ev, v}^{d, j}T_{s}}{E_{v}^{cap}}), \qquad \forall v \in [0,V] $$
      
      $$ SOC_{ev}^{threshold} \leq SOC_{ev, v}^{j-1} + \sum_{j=\tau_{ev, v}^{s}}^{\tau_{ev, v}^{e}} (\frac{\eta_{v}^{c}P_{v}^{c, max}r_{ev, v}^{c,j}T_{s}}{E_{v}^{cap}} - \frac{\eta_{v}^{d}P_{v}^{d, max}r_{ev, v}^{d, j}T_{s}}{E_{v}^{cap}}), \qquad \forall v \in [0,V] $$

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
}}
\sum_{j=k}^{J-1} (\rho_{b}^{j}P^{j}_{u,grid} T_{s}+vL_{u}^{j}) -
\sum_{j=\tau_{r}}^{\tau_{r}^{e}} \alpha_{u}^{j}\rho_{f}^{j}(\bar{P}_{u, base}-P^{j}_{u,grid}) T_{s},
\quad if \quad k<\tau_{r}^{s}
$$

$$ \min_{\substack{
    r_{u,a}^{j},~j=0,...,J-1,~a \in A_{u, c1} \cup A_{u, c2} \cup A_{u, c3}\\ 
    \delta_{u,a}^{j},~j=0,...,J-1,~a \in A_{u, c2} \cup A_{u, c3}\\ 
    P_{u, a}^{j},~j=0,...,J-1,~a \in A_{u, c3}\\
    P_{u, grid}^{j},~j=0,...,J-1\\
}}
\sum_{j=k}^{J-1} (\rho_{b}^{j}P^{j}_{u,grid} T_{s}+vL_{u}^{j}) -
\sum_{j=k}^{\tau_{r}^{e}} \alpha_{u}^{j}\rho_{f}^{j}(\bar{P}_{u, base}-P^{j}_{u,grid}) T_{s},
\quad if \quad \tau_{r}^{s}\leq k \leq \tau_{r}^{e} 
$$

$$ \min_{\substack{
    r_{u,a}^{j},~j=0,...,J-1,~a \in A_{u, c1} \cup A_{u, c2} \cup A_{u, c3}\\ 
    \delta_{u,a}^{j},~j=0,...,J-1,~a \in A_{u, c2} \cup A_{u, c3}\\ 
    P_{u, a}^{j},~j=0,...,J-1,~a \in A_{u, c3}\\
    P_{u, grid}^{j},~j=0,...,J-1\\
}}
\sum_{j=k}^{J-1} (\rho_{b}^{j}P^{j}_{u,grid} T_{s}+vL_{u}^{j})
\quad if \quad  k>\tau_{r}^{e}\\ 
$$
<!-- HEMS Constraint -->
* Comfort level
  
  $$ w_{u,a}^{j}=
  \left\{ 
      \begin{array}
        r\frac{j-\tau_{u,a}^{s_{i},c}}{\tau_{u,a}^{e_{i},c}-\tau_{u,a}^{s_{i},c}}+(c+1),if\quad & j\in[\tau_{u,a}^{s_{i},c},\tau_{u,a}^{e_{i},c}],i=1...m_{c},c=1...n_{a}\\
        \infty \qquad, \qquad &otherwise
      \end{array}
    \right.
  $$

  $$ L_{u}^{j} = \sum_{a \in A_{c1} \cup A_{c2} \cup A_{c3}}w_{u,a}^{j}r_{u,a}^{j} $$
* Demand response
    
	* $\alpha_{u}^{j}$: 住戶u在j時刻是否參與輔助服務
    * Formula

    $$ \alpha_{u}^{j} \in \{0, 1\} $$

    $$ \bar{P}_{u, base} = \frac{1}{N}\frac{1}{(\tau_{r}^{e} - \tau_{r}^{s})}\sum_{n=1}^{N}\sum_{j=\tau_{r}^{s}}^{\tau_{r}^{e}} P_{u, -n}^{j} $$

* Balanced Function
	* $A_{u,uc}$第u個住戶的不可控負載類型
	* $P_{u,uc}^{j}$第u個住戶在第j個時刻的不可控負載功率

    $$ P_{u,grid}^{j} = \sum_{a \in A_{u, c1} \cup A_{u, c2} \cup A_{u, c3}} P_{u,a}^{j}  + \sum_{a \in A_{u, uc}} P_{u,uc}^{j}$$

* Pgrid
    * Variable 
  
    $$ 0 \leq P_{u,grid}^{j} \leq
    \left\{ 
        \begin{array}{c}
          \bar{P}_{u, base}, &if \quad \forall j \in [\tau_{r}^{s}, \tau_{r}^{e}] \, and \, \alpha_{u}^{j} = 1\\
          P_{u,grid}^{max}, &otherwise
        \end{array}
      \right.
    $$
    

* Loads Constraint
  <!-- Uncontrollable load Constraint -->
  * Uncontrollable load
    $$ P_{u,uc}^{j} =
    \left\{ 
      \begin{array}
        rr_{u,uc}^{j} P_{u,uc}^{}, \qquad &\forall j \in [\tau_{u}^{s}, \tau_{u}^{e}]\\
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
        rr_{u,a}^{j} P_{u,a}^{max}, \qquad &\forall j \in [\tau_{u,a}^{s}, \tau_{u,a}^{e}]\\
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

* $T_{u}^{total}$ 各住戶原始總花費
  $$ T_{u}^{total} = \sum_{j=0}^{J-1}(\frac{P_{u, grid}^{j}}{\sum_{u=1}^{U}P_{u, grid}^{j}} \times T^{j, cost}) $$

* $O_{u}^{cost}$ 各住戶最佳化市電花費
  $$ O_{u}^{cost} = \sum_{j=0}^{J-1}(\frac{T_{u, h}^{j}}{\sum_{u=1}^{U}T_{u, h}^{j}} \times O^{j, cost}) $$

* $O_{u, dr}^{feedback}$ 各住戶輔助服務回饋
  $$ C_{u}^{j} = \alpha_{u}^{j}\rho_{f}^{j}(\bar{P}_{u,base}-P_{u, grid}^{j})T_{s} $$
  $$ O_{u, dr}^{feedback} = \sum_{j=\tau_{r}^{s}}^{\tau_{r}^{e}}(\frac{ C_{u}^{j}}{\sum_{u=1}^{U}C_{u}^{j}} \times O_{dr}^{j,feedback}) $$

* $O_{u}^{total}$ 各住戶最佳化總花費
  $$ O_{u}^{total} = O_{u}^{cost} - O_{u, dr}^{feedback} $$

* $\eta_{u}^{cost}$ 各住戶節省電費比
  $$ \eta_{u}^{cost} = \frac{T_{u}^{total}-O_{u}^{total}}{T_{u}^{total}} $$

##### CEMS
* $R^{j, cost}$, $R^{cost}$ 社區負載花費
  $$ R^{j, cost} = \rho^{j}P_{load}^{j}T_{s} $$
  $$ R^{cost} = \sum_{j=0}^{J-1}T^{j, cost} $$

* $O^{j, cost}$, $O^{cost}$ 社區總市電花費
  $$ O^{j, cost} = \rho^{j}P_{grid}^{j}T_{s} $$
  $$ O^{cost} = \sum_{j=0}^{J-1}O^{j, cost} $$

* $O_{dr}^{j, feedback}$, $O_{dr}^{feedback}$ 社區輔助服務回饋
  $$ O_{dr}^{j, feedback} = \rho_{f}^{j} (\bar{P}_{base}-P^{j}_{grid}) T_{s} $$
  $$ O_{dr}^{feedback} = \sum_{j=\tau_{r}^{s}}^{\tau_{r}^{e}}O_{dr}^{j, feedback} $$

* $O_{comm}^{total}$ 社區最終花費
  $$ O_{comm}^{total} = O^{cost} - O_{dr}^{feedback} $$