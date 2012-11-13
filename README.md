cmput411-assignment3
====================

This program reads in and displays a model of a human in the wavefront obj format and a skeleton animation in the .bvh format. The skeleton animation may be played back and the model will morph along with it! See the pdf description of the assignment for full features.

(Note that while technically other models can also be loaded, they need to be matched up to the skeleton for the program to work correctly.)

Development was done in eclipse, hence this folder actually can be imported into eclipse as a project rather simply, and then built and run there. (Do not forget to specify which model to load in the command line arguments.)

Alternatively, the code can be compiled and then run with the following commands (THIS DOES NOT WORK YET!):
```
make
./personviewer <meshfile.obj> <motiongile.bvh>
```

###### Assumptions about the project
1. All the bvh files we load either have "CHANNELS 6 Xposition Yposition Zposition Zrotation Yrotation Xrotation" or "CHANNELS 3 Zrotation Yrotation Xrotation"
2. Any root is not a leaf (this is valid assumption as that would not make sense)
3. I do NOT assume that there's only one root in the file, however I never tested whether my parser works with multiple roots or not. In case there are multiple roots, I assume that their animation descriptions are interleaved, as in the first line contains the information about the first frame for all the trees, then the second line describes the second frame for all trees, etc.
4. For calculating the initial viewing position I make the assumption that only ROOT's have translation transformations.
5. When outputting the .obj file, I output the normalized normals.
6. Each face is in the .obj model is a triangle.
7. While closest bone to vertex should intuitively be:
<pre>	argmin_{bone b} ( inf_{line l connecting a point of b to v, not crossing any faces} (length of l) )</pre>
This is hard to calculate and so i do
<pre>	argmin_{bone b, the line segment l joining closest point of b with v does not cross any faces} ( length of l )</pre>
as suggested in the notes.


--------------------
Developed by David Szepesvari. Started November 8, 2012.