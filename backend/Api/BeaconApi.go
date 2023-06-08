package Api

type BeaconChargeStruct struct {
	SessionId    	  	string  `json:"SessionId"`
	Charged      		string  `json:"Charged"`
}

type BeaconOnStruct struct {
	SessionId    	  	string  `json:"SessionId"`
	On      			string  `json:"On"`
}

type BeaconConnectedStruct struct {
	SessionId    	  	string  `json:"SessionId"`
	LastSeen      		string  `json:"On"`
}