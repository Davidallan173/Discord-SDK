import time
from ctypes import * # Used to interact with DLL
from direct.task import Task # Used to manage the Callback timer

class DiscordRPC:

    def __init__(self):
        self.CodeHandle = cdll.LoadLibrary("SDK.dll") # Load the RP code
        self.CodeHandle.DLLMain()
        self.UpdateTask = None
        self.details = "Loading" # The writing next to the photo
        self.image = "logo" #The main photo
        self.imageTxt = "Loading " # Hover text for the main photo
        self.smallLogo = "" # Small photo in corner
        self.smallTxt = "" # Text on hover of that photo
        self.state = "" # Displayed underneath details - used for lobbys etx
        self.PartySize = 0
        self.MaxParty = 0

    def stopLobby(self):  #Boarding groups :D
        self.PartySize = 0
        self.state = ""
        self.MaxParty = 0
        self.setData()

    def AllowLobby(self, size, desc):
        self.state = description
        self.PartySize = 1
        self.MaxParty = size
        self.setData()

    def setLobbySize(self, size): # Sets how many members are in a boarding group
        self.PartySize = size
        self.setData()

    def setData(self): # Manually update all vars
        self.CodeHandle.DoCallbacks()
        details = self.details
        image = self.image
        imageTxt = self.imageTxt
        smallLogo = self.smallLogo
        smallTxt = self.smallTxt
        state = self.state
        party = self.PartySize
        maxSize = self.MaxParty # Discord uses UTF-8 encoded strings
        self.CodeHandle.SetData(details.encode('utf_8'), state.encode('utf_8'), smallLogo.encode('utf_8'), smallTxt.encode('utf_8'), image.encode('utf_8'), imageTxt.encode('utf_8'), maxSize, party)

    def DoCallbacks(self, task): # Recieves any messages from discord and handles them
        self.CodeHandle.DoCallbacks()
        return task.again

    def UpdateTasks(self, task):
        self.UpdateTask = True
        self.setData()
        return task.again

    def SetImageData(self, image, details):
        self.image = image
        self.details = details
        self.setData()

    def StartTasks(self): # Call AFTER Showbase has been initialised
        taskMgr.doMethodLater(10, self.UpdateTasks, 'UpdateTask')
        taskMgr.doMethodLater(0.016, self.DoCallbacks, 'RPC-Callbacks')
