import clr
clr.AddReference("PresentationFramework")
clr.AddReference("PresentationCore")
clr.AddReference('System.Drawing')

import Queue, cmath, math
from System.IO import File
from System.Drawing import ContentAlignment
from System.Windows.Markup import XamlReader
from System.Windows import (
    Application, Window, HorizontalAlignment, VerticalAlignment, Shapes, Visibility, MessageBox
)
from System.Windows.Controls import (
    Label, TextBlock, Grid, RowDefinition, ColumnDefinition 
)

# ------ Config Section --------- #
startTileHalf = True    # First tile is half a tile
cols = 6                # The # of columns
rows = 12               # The # of rows
# =============================== #

class Lab5(object):
    path = []

    def buildLabels(self):
	for r in range(rows):
	    for c in range(cols):
		label = Label()
		label.Name = "and{0}and{1}".format(r,c)
		label.HorizontalAlignment = HorizontalAlignment.Center
		label.VerticalAlignment = VerticalAlignment.Center
		
		label.MouseLeftButtonUp += self.onClickTarget
		self.grid.Children.Add(label)
		
		Grid.SetRow(label, r)
		Grid.SetColumn(label, c)
		
		# Reset the path
		self.path = []

    # Clear the label contents
    def onClickClear(self, sender, event):
	# Clear and rebuild
	self.grid.Children.Clear()
	self.buildLabels()
	
    # Clear the label contents
    def onClickStart(self, sender, event):
	# Start the movement
	s = str()
	for i in range(len(self.path)-1):
	    #s += "{0} - Row {1}, Column {2}\n".format(i, self.path[i][0], self.path[i][1])
	    # Compare two taregst to see what movement needs to be performed
	    horizontal = self.path[i+1][1] - self.path[i][1]
	    vertical = self.path[i+1][0] - self.path[i][0]
	    
	    # One of three movements can occur
	    # 1 - Moving Horizontally
	    # 2 - Move Vertically
	    # 3 - Move Diagonally
	    
	    if self.path[i+1][0] == self.path[i][0]:
		# On the same row, move horizontally
		# Check if we must move backwards or forwards
		if horizontal > 0 or i == 0:
		    s += "Move forward {0} tiles\n".format(abs(horizontal))
		else:
		    # Turn 180 degrees and move forward
		    s += "Turn 180 degrees Counter clockwise and move forward {0} tiles\n".format(abs(horizontal))
	    elif self.path[i+1][1] == self.path[i][1]:
		# On the same column, move vertically
		if vertical > 0:
		    s += "Turn 90 degrees clockwise and move forward {0} tiles\n".format(vertical)
		else:
		    # Turn 90 degrees CCW and move forward
		    s += "Turn 90 degrees counter clockwise and move forward {0} tiles\n".format(abs(vertical))
	    else:
		# Move vertically
		# calculate the diagonal to move
		hypotenuse = math.sqrt( pow(abs(horizontal),2) +  pow(abs(vertical),2))
		angle = math.degrees(math.asin( abs(vertical)/hypotenuse ))
		
		if vertical > 0 and horizontal > 0:
		    s += "Turn {0} degrees clockwise and move forward {1} tiles\n".format(angle, hypotenuse)
		elif vertical > 0 and horizontal < 0:
		    s += "Turn {0} degrees clockwise and move forward {1} tiles\n".format(180-angle, hypotenuse)
		elif vertical > 0 and horizontal > 0:
		    s += "Turn {0} degrees counter clockwise and move forward {1} tiles\n".format(180-angle, hypotenuse)
		else:
		    s += "Turn {0} degrees counter clockwise and move forward {1} tiles\n".format(angle, hypotenuse)
		    
	# Display result
	MessageBox.Show(s)

    def __init__(self):
	stream = File.OpenRead("Main.xaml")
	self.Root = XamlReader.Load(stream)
	
	# Instantiate form objects
	self.grid = self.Root.FindName("grid")
	self.startButton = self.Root.FindName("startButton")
	self.clearButton = self.Root.FindName("clearButton")
	
	# Add handlers
	self.startButton.Click += self.onClickStart
	self.clearButton.Click += self.onClickClear

	# Create row and Column Definitions
	for c in range(cols):
	    self.grid.ColumnDefinitions.Add( ColumnDefinition() )

	for r in range(rows):
	    self.grid.RowDefinitions.Add( RowDefinition() )

	# Add Labels in all cells
	self.buildLabels()

    # Adds the target
    def onClickTarget(self, sender, event):
	# Check to see if content was already set
	if sender.Content == None:
	    # Insert an Ellipse in the target label
	    grid = Grid()
	    
	    ellipse = Shapes.Ellipse()
	    ellipse.Style = Application.Current.Windows[0].Resources["TargetStyle"]
	    grid.Children.Add(ellipse)
	    sender.Content = grid
	    
	    text = TextBlock()
	    text.Style = Application.Current.Windows[0].Resources["TextBlockStyle"]
	    text.Text = str(len(self.path)+1)
	    grid.Children.Add(text)
	    
	    ellipse.Visibility = Visibility.Visible
	    
	    # Update moveMatrix position
	    rowCol = sender.Name.split("and")
	    lastRowCol = [int(rowCol[1]), int(rowCol[2])]
	    self.path.append([lastRowCol[0], lastRowCol[1]])

lab5 = Lab5()
app = Application()
app.Run(lab5.Root)