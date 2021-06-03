#!/bin/bash
truncate -s 0 ~/how/fifty_DHEMS/log/*.log

start_time=`date "+%Y-%m-%d %H:%M:%S"`
for i in {1..96}
do
   for j in {1..5}
   do
      echo "-------- RUN LHEMS at $j times --------"
      /home/hems/how/fifty_DHEMS/build/LHEMS    >> /home/hems/how/fifty_DHEMS/log/LHEMS.log  &
      /home/hems/how/fifty_DHEMS/build/LHEMS2   >> /home/hems/how/fifty_DHEMS/log/LHEMS2.log &
      /home/hems/how/fifty_DHEMS/build/LHEMS3   >> /home/hems/how/fifty_DHEMS/log/LHEMS3.log &
      /home/hems/how/fifty_DHEMS/build/LHEMS4   >> /home/hems/how/fifty_DHEMS/log/LHEMS4.log &
      /home/hems/how/fifty_DHEMS/build/LHEMS5   >> /home/hems/how/fifty_DHEMS/log/LHEMS5.log &
      /home/hems/how/fifty_DHEMS/build/LHEMS6   >> /home/hems/how/fifty_DHEMS/log/LHEMS6.log &
      /home/hems/how/fifty_DHEMS/build/LHEMS7   >> /home/hems/how/fifty_DHEMS/log/LHEMS7.log &
      /home/hems/how/fifty_DHEMS/build/LHEMS8   >> /home/hems/how/fifty_DHEMS/log/LHEMS8.log &
      /home/hems/how/fifty_DHEMS/build/LHEMS9   >> /home/hems/how/fifty_DHEMS/log/LHEMS9.log &
      /home/hems/how/fifty_DHEMS/build/LHEMS10  >> /home/hems/how/fifty_DHEMS/log/LHEMS10.log 
      wait
   done
   wait
   echo "~~~~~~~~~~ RUN GHEMS at $i times ~~~~~~~~~~"
   /home/hems/how/fifty_DHEMS/build/GHEMS $Hydro_Price $weather >> /home/hems/how/fifty_DHEMS/log/GHEMS.log
done
end_time=`date "+%Y-%m-%d %H:%M:%S"`
duration=`echo $(($(date +%s -d "${end_time}") - $(date +%s -d "${start_time}"))) | awk '{t=split("60 s 60 m 24 h 999 d",a);for(n=1;n<t;n+=2){if($1==0)break;s=$1%a[n]a[n+1]s;$1=int($1/a[n])}print s}'`

echo -e "\nTime cost: $duration"