echo Script to convert the demo mp4 to an animated gif

SET mp4gif="D:\dev\ed-wares\Mp4gif\distrib\mp4gif\mp4gif.exe"
%mp4gif% DemoCubey.mp4 DemoCubey.gif "fps=30,scale=480:-1:flags=lanczos,split[s0][s1];[s0]palettegen=max_colors=32[p];[s1][p]paletteuse=dither=bayer"
