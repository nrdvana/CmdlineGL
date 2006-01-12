# Title: Timing.sh
# Project: Galactic Avenger
# Author: Michael Conrad
# Date: 2005-06-21
#
if [[ -z "$LIB_TIMING" ]]; then
LIB_TIMING=1;

# Timing API
#-------------------------------
# SetMinMaxFPS(min,max: int)
# UpdateTime(time: int)
# SyncNextFrame()
# GetFPSString()
#
# Timing_T: milliseconds - virtual time
# Timing_RT: milliseconds - real time
# Timing_dT: milliseconds - virtual time elapsed since last frame
# Timing_FPS: int - frames per real second
# Timing_FPVS: int - frames per virtual second
# Timing_AvgCount: int - number of samples to average
# Timing_MinFPS: int - lowest allowed FPVS
# Timing_MaxFPS: int - highest allowed FPS
#

SetMinMaxFPS() {
	local NewMin=$1
	local NewMax=$2
	(( Timing_MinFPS=NewMin ))
	(( Timing_MaxFPS=NewMax ))
	(( Timing_MaxFPS < Timing_MinFPS )) && (( Timing_MaxFPS=Timing_MinFPS ))
	(( Timing_MaxFPS < 1 )) && Timing_MaxFPS=1
	(( Timing_Min_dT=1000/Timing_MaxFPS ))
	if (( Timing_MinFPS > 0 )); then
		(( Timing_Max_dT=(1000+Timing_MinFPS/2)/Timing_MinFPS )) # round up
	else
		Timing_Max_dT=1000000;
	fi
}

UpdateTime() {
	local NewTime=$1
	(( Timing_dT=NewTime-Timing_RT ))
	(( Timing_dT<1? Timing_dT=1 : 1))
	(( Timing_dRTAvg= (Timing_dRTAvg * Timing_AvgCount + Timing_dT*100) / (Timing_AvgCount+1) ))
	(( Timing_dT>Timing_Max_dT? Timing_dT=Timing_Max_dT : 1))
	(( Timing_RT=NewTime, Timing_T+=Timing_dT ))
	(( Timing_dTAvg= (Timing_dTAvg * Timing_AvgCount + Timing_dT*100) / (Timing_AvgCount+1) ))

	(( Timing_FPVS=100000/Timing_dTAvg, Timing_FPS=100000/Timing_dRTAvg ))
}

SyncNextFrame() {
	cglSync $(( Timing_RT+Timing_Min_dT ))
}

# Initialization
#

SetMinMaxFPS 10 70

Timing_T=0;
Timing_RT=0;
Timing_Slip=0;

Timing_dT=0;
Timing_dTAvg=100;
Timing_dRTAvg=100;
Timing_AvgCount=10;
Timing_FPS=0;
Timing_FPVS=0;

fi
