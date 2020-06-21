class NetPorts:
    def __init__(self,sip_port="",rtp_port="",ctrl_port="",src_ip = ""):   
        self.setCfg(sip_port,rtp_port,ctrl_port,src_ip)
    defnit = ""
    def setCfg(self,sip_port,rtp_port,ctrl_port,src_ip):
        self.local_ip    = str(src_ip)
        self.sip         = str(sip_port)
        self.rtp         = str(rtp_port)
        self.ctrl         = str(ctrl_port)
    def getCfg(self):
        return self.__dict__