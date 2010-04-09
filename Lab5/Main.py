import clr
clr.AddReference("PresentationFramework")
clr.AddReference("PresentationCore")
clr.AddReference('System.Drawing')

from System.IO import File
from System.Drawing import ContentAlignment
from System.Windows.Markup import XamlReader
from System.Windows import (
    Application, Window, HorizontalAlignment
)
from System.Windows.Controls import Label

class Lab5(object):
    def __init__(self):
        stream = File.OpenRead("Main.xaml")
        self.Root = XamlReader.Load(stream)
        self.button = self.Root.FindName("button")
        self.stackPanel = self.Root.FindName("stackPanel")
        self.button.Click += self.onClick
        
    def onClick(self, sender, event):
        message = Label()
        message.HorizontalAlignment = HorizontalAlignment.Center
        message.FontSize = 36
        message.Content = "Lab 5"
        self.stackPanel.Children.Add(message)
        
lab5 = Lab5()
app = Application()
app.Run(lab5.Root)