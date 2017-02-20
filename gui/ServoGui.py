#!/usr/bin/python
from PIL import Image, ImageTk
from Tkinter import Tk, Label, BOTH, Y,LEFT,RIGHT,X,RAISED,SUNKEN,W,StringVar,END, DISABLED, NORMAL, TOP
import tkFileDialog as fD
from ttk import Frame, Style, Button, Entry, OptionMenu
import serial
import serial.tools.list_ports
import threading
import Queue
import time
import matplotlib
import numpy

matplotlib.use('TkAgg')
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
# implement the default mpl key bindings
from matplotlib.backend_bases import key_press_handler


from matplotlib.figure import Figure
drivesettings = [
    "commutationMethod",
    "inputMethod",
    "encoder_PPR",
    "encoder_poles",
    "encoder_counts_per_step",
    "pid_Kp",
    "pid_Ki",
    "pid_Kd",
    "pid_FF1",
    "pid_FF2",
    "usart_baud",
    "max_current",
    "max_error",
    "invert_dirstepena"
    ]

class SerialThread(threading.Thread):
    def __init__(self, queue, writequeue, ser):
        threading.Thread.__init__(self)
        self.ser = ser
        self.queue = queue
        self.writequeue = writequeue
        self.buffer = ""
    def stop(self):
        self.running = False
    def run(self):
        self.running = True
        while self.running:
            while self.ser.inWaiting():
                ch = self.ser.read(1)

                if ch == '\n' or ch == '\r':
                    self.queue.put(self.buffer)
                    self.buffer = ""
                else:
                    self.buffer += ch
            while self.writequeue.qsize():
                try:
                    line = self.writequeue.get()
                    self.ser.write(line)
                except Queue.Empty:
                    pass


class topFrame(Frame):
    def __init__(self, parent):
        Frame.__init__(self, parent)
        self.parent = parent
        self.setUI()

    def setUI(self):
        self.parent.title("ServoGui")
        self.pack(fill=BOTH, expand=1)
        self.comPort = StringVar(self)
        self.laststrm = StringVar(self)

        settingFrame = Frame(self, borderwidth=1, relief=RAISED)
        settingFrame.pack(fill=Y, side=LEFT)

        Label(settingFrame, width=50, text="Port Settings", bg="green", fg="black").pack(fill=X)

        ports = self.getComPorts()
        w = apply(OptionMenu, (settingFrame, self.comPort) + tuple(ports))
        w.pack(fill=X)

        BaudFrame = Frame(settingFrame)
        BaudFrame.pack(fill=X)
        Label(BaudFrame, text="Baud:").pack(side=LEFT)
        self.baud_entry = Entry(BaudFrame,
                                width=15,
                                validate="focusout",
                                validatecommand=self.baudValidate)
        self.baud_entry.pack(side=LEFT, expand = True)
        self.baud_entry.insert(0,"115200")

        Button(settingFrame, text="Open Port", command=self.openPort). pack(fill=X)
        Button(settingFrame, text="Close Port", command=self.closePort). pack(fill=X)

        StreamFrame = Frame(settingFrame)
        StreamFrame.pack()
        self.btnStartStream = Button(StreamFrame,
                                text="Start Stream",
                                command=self.startStream,
                                state=DISABLED)
        self.btnStopStream = Button(StreamFrame,
                                text="Stop Stream",
                                command=self.stopStream,
                                state=DISABLED)
        self.btnGetConfig = Button(StreamFrame,
                                text="Get Config",
                                command=self.getConfig,
                                state=DISABLED)
        self.btnStartStream.pack(side=LEFT)
        self.btnStopStream.pack(side=LEFT)
        self.btnGetConfig.pack(side=LEFT)
        self.queue = Queue.Queue()
        self.writequeue = Queue.Queue()

        Label(settingFrame, width=50, text="Drive Settings", bg="green", fg="black").pack(fill=X)
        DriveSettingsFrame = Frame(settingFrame, relief=SUNKEN)
        DriveSettingsFrame.pack(fill=X)

        driveSettingsFrames = []
        self.driveSettingsEntries = []
        for drivesetting in drivesettings:
            driveSettingsFrames.append(Frame(DriveSettingsFrame))
            driveSettingsFrames[-1].pack(fill=X)
            Label(driveSettingsFrames[-1], text=drivesetting).pack(side=LEFT)
            self.driveSettingsEntries.append(Entry(driveSettingsFrames[-1]))
            self.driveSettingsEntries[-1].pack(side=RIGHT)
        Button(DriveSettingsFrame, text="Send to drive", command=self.sendConfig).pack(fill=X)
        Button(DriveSettingsFrame, text="Save config in drive", command=self.saveConfig).pack(fill=X)

        Label(settingFrame, width=50, textvariable=self.laststrm, bg="green", fg="black").pack(fill=X)

        #MatplotLib stuff

        f = Figure(figsize=(5, 4), dpi=100)
        self.a = f.add_subplot(311)
        self.a.set_title("Requested and actual position")
        self.b = f.add_subplot(312)
        self.b.set_title("Error")
        self.c = f.add_subplot(313)
        self.c.set_title("Current meas ADC value")

        self.canvas = FigureCanvasTkAgg(f, master=self)
        self.canvas.show()
        self.canvas.get_tk_widget().pack(side=TOP, fill=BOTH, expand=1)

        toolbar = NavigationToolbar2TkAgg(self.canvas, self)
        toolbar.update()
        self.canvas._tkcanvas.pack(side=TOP, fill=BOTH, expand=1)

        self.hall=[]
        self.encoder_count=[]
        self.pos_error=[]
        self.requested_position=[]
        self.requested_delta=[]
        self.adc_value=[]
        self.pid_output=[]
        self.a.set_autoscaley_on(True)


        self.encoder_line, = self.a.plot([],[])
        self.error_line, = self.b.plot([],[])
        self.reqpos_line, = self.a.plot([],[])
        self.ADC_line, = self.c.plot([],[])
        self.updateCanvas()


    def baudValidate(self):
        sVal = self.baud_entry.get()
        try:
            iVal = int(sVal)
        except ValueError:
            print "Illegal baud value"
            self.baud_entry.delete(0, END)
            self.baud_entry.insert(0, "115200")
            return False
        return True

    def openPort(self):
        try:
            self.ser = serial.Serial(self.comPort.get(), int(self.baud_entry.get()), timeout=0)
        except serial.SerialException:
            print "unable to open"
            return
        self.btnStartStream['state'] = NORMAL
        self.btnStopStream['state'] = NORMAL
        self.btnGetConfig['state'] = NORMAL

        self.thread = SerialThread(self.queue, self.writequeue, self.ser)
        self.thread.daemon = True
        self.thread.start()
        self.process_serial()

    def closePort(self):
        self.thread.stop()
        self.thread.join()
        self.ser.closePort()

        self.btnStartStream['state'] = DISABLED
        self.btnStopStream['state'] = DISABLED
        self.btnGetConfig['state'] = DISABLED

    def process_serial(self):
        while self.queue.qsize():
            try:
                line = self.queue.get()
                self.handleLine(line)
            except Queue.Empty:
                pass
        self.after(100, self.process_serial)

    def startStream(self):
        self.writequeue.put(b"STREAM START \r")

    def stopStream(self):
        self.writequeue.put(b"STREAM DIE \r")
    def getConfig(self):
        self.writequeue.put(b"GET\r")
    def saveConfig(self):
        self.writequeue.put(b"SAVE \r")
    def sendConfig(self):
        for setting in drivesettings:
            dataToSend = b"SET "+setting+" "+self.driveSettingsEntries[drivesettings.index(setting)].get()+"\r"
            print dataToSend
            self.writequeue.put(dataToSend)
            time.sleep(0.2)


    def getComPorts(self):
        ports = serial.tools.list_ports.comports()
        portNames = []
        for port in ports:
            portNames.append(port[0])
        return portNames
    def handleLine(self,line):
        line = line.replace(" ", "")
        line = line.replace("/n", "")
        line = line.replace("/r", "")
        parts = line.split(":")
        if len(parts)>1:
            if parts[0] == "STR":
                self.handleStr(parts[1])
                return
            if parts[0] in drivesettings:
                self.driveSettingsEntries[drivesettings.index(parts[0])].delete(0, END)
                self.driveSettingsEntries[drivesettings.index(parts[0])].insert(0, parts[1])
    def handleStr(self,strm):
        #format of the stream line: STR:hall;count;requestedPosition;requestedDelta;error
        parts = strm.split(";")

        self.laststrm.set(strm)
        self.hall.append(int(parts[0]))
        if len(self.hall) > 5000:
            self.hall.pop(0)

        self.encoder_count.append(parts[1])
        if len(self.encoder_count) > 5000:
            self.encoder_count.pop(0)

        self.requested_position.append(parts[2])
        if len(self.requested_position) > 5000:
            self.requested_position.pop(0)

        self.requested_delta.append(parts[3])
        if len(self.requested_delta) > 5000:
            self.requested_delta.pop(0)

        self.pos_error.append(parts[4])
        if len(self.pos_error) > 5000:
            self.pos_error.pop(0)

        self.adc_value.append(parts[5])
        if len(self.adc_value) > 5000:
            self.adc_value.pop(0)

        self.pid_output.append(parts[5])
        if len(self.pid_output) > 5000:
            self.pid_output.pop(0)

    def updateCanvas(self):

        self.encoder_line.set_xdata(range(len(self.encoder_count)))
        self.encoder_line.set_ydata(self.encoder_count)
        self.error_line.set_xdata(range(len(self.pos_error)))
        self.error_line.set_ydata(self.pos_error)
        self.reqpos_line.set_xdata(range(len(self.requested_position)))
        self.reqpos_line.set_ydata(self.requested_position)
        self.ADC_line.set_xdata(range(len(self.adc_value)))
        self.ADC_line.set_ydata(self.adc_value)
        self.a.relim()
        self.a.autoscale_view()
        self.b.relim()
        self.b.autoscale_view()
        self.c.relim()
        self.c.autoscale_view()
        self.canvas.draw()
        self.after(100, self.updateCanvas)




def main():
    root = Tk()
    root.geometry("300x280+300+300")
    app = topFrame(root)
    root.mainloop()

if __name__ == '__main__':
    main()

