import clr
clr.AddReference("System.IO.Ports")

baudRate = 9600
dataBits = 8
stopBit = 1

class SerialCom:
    
    self.serialConnection = None;
    # self.serialBuffer = ''
    self.comPort = 8
    
    def connect(self, comNum = None):
        # Setup default connection parameters
        if comNum == None: # Use the default communication port
            self.serialConnection = SerialPort( "COM" + self.comPort)
        else :self.serialConnection = SerialPort("COM" + comNum)
        self.serialConnection.BaudRate = baudRate
        self.serialConnection.DataBits = dataBits
        self.serialConnection.StopBit = stopBit
        # Connect to the remote device
        #self.serialConnection.Close() # in the case where a connection is already open
        self.serialConnection.Open()
        return
    
    def disconnect(self):
        self.serialConnection.Close()
        self.serialConnection = None
        return
    
    def send(self, msg):
        self.serialConnection.WriteLine(msg)
        return
    
    def receivedMsg(self):
        return self.serialConnection.ReadLine()

    def isConnected(self):
        return self.serialConnection != None