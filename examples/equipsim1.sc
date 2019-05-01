let sc_version = get_version()

print "SC Version is <" . $sc_version . ">"

#let host_ip_address = "10.100.32.140"
let host_ip_address = "192.168.100.24"


let E5Model = "EqModel"
let E5SoftwareVersion = "1.2.14"

# SVIDs
let AlarmsEnabled = 1
let AlarmsSet = 0
let Clock = "2017011400183400"
let ControlState = 5
let EventsEnabled = 1
let PPErrorSV = ""
let PPExecNameFormat = 32
let PPFormat = ""
let PreviousProcessState = ""
let ProcessState = ""
let RcpExecName = ""
let SpoolCountActual = 0
let SpoolCountTotal = 0
let SpoolFullTime = "2006080100183400"
let SpoolStartTime = "2006080100183400"

#ECID
let EstablishCommunicationsTimeout = 10
let MaxSpoolTransmit = 0
let OverWriteSpool = 1
let TimeFormat = "?"

#DVIDs
let AlarmID = 1
let EventLimit = 10
let LimitVariable = 10
let PPChangeName = ""
let PPChangeStatus = ""
let PPErrorDV = ""
let TransitionType = ""

define_xref vid = E5Model, 1
define_xref vid = E5SoftwareVersion, 2

define_xref vid = AlarmsEnabled, 1001
define_xref vid = AlarmsSet, 1002
define_xref vid = Clock, 1003
define_xref vid = ControlState, 1004
define_xref vid = EventsEnabled, 1005
define_xref vid = PPErrorSV, 1006
define_xref vid = PPExecNameFormat, 1007
define_xref vid = PPFormat, 1008
define_xref vid = PreviousProcessState, 1009
define_xref vid = ProcessState, 1010
define_xref vid = RcpExecName, 1011
define_xref vid = SpoolCountActual, 1012
define_xref vid = SpoolCountTotal, 1013
define_xref vid = SpoolFullTime, 1014
define_xref vid = SpoolStartTime, 1015

define_xref vid = EstablishCommunicationsTimeout, 2001
define_xref vid = MaxSpoolTransmit, 2002
define_xref vid = OverWriteSpool, 2003
define_xref vid = TimeFormat, 2004

define_xref vid = AlarmID, 3001
define_xref vid = EventLimit, 3002
define_xref vid = LimitVariable, 3003
define_xref vid = PPChangeName, 3004
define_xref vid = PPChangeStatus,3005
define_xref vid = PPErrorDV, 3006
define_xref vid = TransitionType, 3007



#E87 Stuff


#DVIDs
let E87PortID_DV = 0;
let E87PortTransferState_DV = 0;
let E87CarrierID_DV = ""
let E87Capacity_DV = 25
let E87CarrierIDStatus_DB = ""
let E87CarrierAccessingStatus_DV = 0

for i = 1 to $E87Capacity_DV
    let E87ContentMap_DV[$i]LotID = ""
    let E87ContentMap_DV[$i]SubstrateID = ""
next i

let E87LocationID_DV = ""
let E87ObjType_DV = "Carrier"
let E87ObjID_DV = ""

for i = 1 to $E87Capacity_DV
    let E87SlotMap_DV[$i] = 0
next i

let E87SlotMapStatus_DV = 0
let E87SubstrateCount_DV = 0
let E87Usage_DV = ""
let E87AccessMode_DV = 0
let E87LoadPortReservationState_DV = 0
let E87LoadPortAssocoationState_DV = 0

define_xref vid = E87PortID_DV, 87001
define_xref vid = E87PortTransferState_DV, 87002
define_xref vid = E87CarrierID_DV, 87003
define_xref vid = E87Capacity_DV, 87004
define_xref vid = E87CarrierIDStatus_DV, 87005
define_xref vid = E87CarrierAccessingStatus_DV, 87006
define_xref vid = E87E87ContentMap_DV, 87007
define_xref vid = E87LocationID_DV, 87008
define_xref vid = E87ObjType_DV, 87009
define_xref vid = E87SlotMap_DV, 87010
define_xref vid = E87SlotMapStatus_DV, 87011
define_xref vid = E87SubstrateCount_DV, 87012
define_xref vid = E87Usage_DV, 87013
define_xref vid = E87AccessMode_DV, 87014
define_xref vid = E87LoadPortReservationState_DV, 87015
define_xref vid = E87LoadPortAssociationState_DV, 87016

#SVIDs
let E87NumberOfPorts = 2

let E87LPT[1]AccessMode_SV = 0
let E87LPT[1]CarrierAccessingStatus_SV = 0
let E87LPT[1]CarrierID_SV = ""
let E87LPT[1]CarrierIDStatus_SV = 0
let E87LPT[1]LocationID[1]_SV = "LPT1"
let E87LPT[1]LocationID[2]_SV = "FIMS1"
let E87LPT[1]LoadPortReservationState_SV = 0
let E87LPT[1]PortAssociationState_SV = 0
let E87LPT[1]PortID_SV = "LPT1"
let E87LPT[1]PortStateInfo = 0
let E87LPT[1]PortTransferState_SV = 0

let E87LPTLoadPortReservationStateList_SV = 0
let E87LPTPortStateInfoList_SV = 0

for i = 1 to $E87NumberOfPorts
    let E87CarrierLocationMatrix_SV[$i]LocationID = ""
    let E87CarrierLocationMatrix_SV[$i]CarrierID = ""
next i

for i = 1 to $E87NumberOfPorts
    let E87LoadPortReservationStateList_SV[$i] = 0
next i

for i = 1 to $E87NumberOfPorts
    let E87LoadPortAssociationStateList_SV[$i] = 0
next i

for i = 1 to $E87NumberOfPorts
    let E87PortStateInfoList_SV[$i]LocationID = ""
    let E87PortStateInfoList_SV[$i]CarrierID = ""
next i

# Resume with port state info...also need to do define_xref for the SVs


define_xref ceid = E87_LPTSM1_NO_STATE_TO_OS_OR_IS, 187001
define_xref ceid = E87_LPTSM2_OS_TO_IS, 187002
define_xref ceid = E87_LPTSM3_IS_TO_OS, 187003
define_xref ceid = E87_LPTSM4_IS_TO_TR_OR_TB, 187004
define_xref ceid = E87_LPTSM5_TR_TO_RTL_OR_RTU, 187005
define_xref ceid = E87_LPTSM6_RTL_TO_TB, 187006
define_xref ceid = E87_LPTSM7_RTU_TO_TB, 187007
define_xref ceid = E87_LPTSM8_TB_TO_RTL, 187008
define_xref ceid = E87_LPTSM9_TB_TO_RTU, 187009
define_xref ceid = E87_LPTSM10_TB_TR, 187010

define_xref ceid = E87_CSM1_NS_TO_CARRIER, 187011
define_xref ceid = E87_CSM2_NS_TO_ID_NOT_READ, 187012
define_xref ceid = E87_CSM3_NS_TO_WFH, 187013
define_xref ceid = E87_CSM4_NS_TO_ID_V_OK, 187014
define_xref ceid = E87_CSM5_NS_TO_ID_V_FAIL, 187015
define_xref ceid = E87_CSM6_ID_NOT_READ_TO_ID_V_OK, 187016
define_xref ceid = E87_CSM7_ID_NOT_READ_TO_WFH, 187017
define_xref ceid = E87_CSM8_WFH_TO_ID_V_OK, 187018
define_xref ceid = E87_CSM9_WFH_TO_ID_V_FAIL, 187019
define_xref ceid = E87_CSM10_ID_NOT_READ_TO_WFH, 187020
define_xref ceid = E87_CSM11_ID_NOT_READ_TO_ID_V_OK, 187021
define_xref ceid = E87_CSM12_NS_TO_SLOTMAP_NOT_READ, 187022
define_xref ceid = E87_CSM13_SLOTMAP_NOT_READ_TO_SLOTMAP_V_OK, 187023
define_xref ceid = E87_CSM14_SLOTMAP_NOT_READ_TO_WFH, 187024
define_xref ceid = E87_CSM15_WFH_TO_SLOTMAP_V_OK, 187025
define_xref ceid = E87_CSM16_WFH_TO_SLOTMAP_V_FAIL, 187026
define_xref ceid = E87_CSM17_NS_TO_NOT_ASCCESSED, 187027
define_xref ceid = E87_CSM18_NOT_ACCESSED_TO_IN_ACCESS, 187028
define_xref ceid = E87_CSM19_IN_ACCESS_TO_CARRIER_COMPLETE, 187029
define_xref ceid = E87_CSM20_IN_ACCESS_TO_STOPPED, 187030
define_xref ceid = E87_CSM21_CARRIER_TO_NS, 187031

define_xref ceid = E87_LPTAMS1_NS_TO_MANUAL_OR_AUTO, 187032
define_xref ceid = E87_LPTAMS2_MANUAL_TO_AUTO, 187033
define_xref ceid = E87_LPTAMS3_AUTO_MANUAL, 187034

define_xref ceid = E87_LPTRS1_NO_TO_NOT_RESEVED, 187035
define_xref ceid = E87_LPTRS2_NOT_RESERVED_TO_RESERVED, 187036
define_xref ceid = E87_LPTRS3_RESERVED_TO_NOT_RESERVED, 187037

define_xref ceid = E87_LPTCASM1_NS_TO_NOT_ASSOCIATED, 187038
define_xref ceid = E87_LPTCASM2_NOT_ASSOCIATED_TO_ASSOCIATED, 187039
define_xref ceid = E87_LPTCASM3_ASSOCIATED_TO_NOT_ASSOCIATED, 187040
define_xref ceid = E87_LPTCASM4_ASSOCIATED_TO_ASSOCIATED, 187041


let client = -1
open socket_server name=host_connection proto=hsms logging=2 host=192.168.100.24 service=5000 options="t3=5 t6=10 t7=10 t8=10 timeout_msg=TIMEOUT" 


# All communications from the host start here
label host_connection
    let client = get_client(host_connection)
    print "Received SECS Message " . $SFXY . " from host."
    let sxfy= get_tmp_var(SXFY)
    goto $sxfy

# S1F1 - Hello from the host
label S1F1
    send_reply name=host_connection S1F2 0
    <L
        <A $E5Model>
        <A $E5SoftwareVersion>
    > .

label S1F3
    #print "Got s1f3 " . get_tmp_var("")
    let x = get_tmp_var("S1F3.1")
    let lbl = "S1F3"
    for i=1 to $x
        let n = "S1F3.1." . $i
        let ivid = get_tmp_var($n)
        print "Status query for <" . $ivid . "> <" . $vid[$ivid] . ">"
        let lbl = $lbl . "_" . $vid[$ivid]
        #let lbl = $lbl . "_" . $ivid
    next i
    let ivid = get_tmp_var("S1F3.1.1")
    print "S1F3 label is <" . $lbl . ">"
    goto $lbl


label S1F3_ControlState
    send_reply name=host_connection S1F4 0
    <L
        <U1 $ControlState>
    >.


# S1F13 - Establish Communications, CommunicationState is COMMUNICATING
label S1F13
    send_reply name=host_connection S1F14 0
    <L
        <B 00>
        <L
            <A $E5Model>
            <A $E5SoftwareVersion>
        >
    >.

# S7F19 - Request PPID List
label S7F19
    send_reply name=host_connection S7F20 0
    <L
        <A '/Test/Recipe001'>
        <A '/Test2/Recipe002'>
        <A 'SZ41T_8a76A'>
        <A 'EMPTY RECIPE'>
    > .

label run_job
    # send job started event
    send name=host_connection client=$client S6F11 0 W
    <L
        <U4 23>
        <U4 5458>
        <L
            <L
                <U4 106>
                <L
                    <A 'CJ001'>
                >
            >
        >
    > .

    sleep 10

    # send job completed event
    send name=host_connection client=$client S6F11 0 W
    <L
        <U4 24>
        <U4 5459>
        <L
            <L
                <U4 106>
                <L
                    <A 'CJ001'>
                >
            >
        >
    > .


