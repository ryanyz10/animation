# TODO
- [x] 1. `F` key saves a keyframe to an ordered list of keyframes
- [x] 2. `P` key plays through the list of keyframes, interpolating between positions
- [x] 3. `R` key rewinds to the beginning of the keyframes
- [x] 4. render keyframes to a texture
- [x] 5. draw the sequence of textures in the right side of the window
- [x] 6. scroll through the keyframe previews with the mouse scroll wheel
- [x] 7. left-clicking keyframe preview selects the keyframe, page up/page down 
- [x] 8. `U` key updates the selected keyframe
- [x] 9. `delete` key deletes the selected keyframe
- [x] 10. `space` sets model pose to pose in keyframe
- [x] 11. `ctrl+s` saves keyframe list to `animation.json`
- [x] 12. Specifying third cli arg reads in animation from `animation.json`
- [x] 13. Popup dialog for saving
- [x] 14. Keyframe cursor to allow arbitrary insertion
- [x] 15. Scrollbar for preview scrolling
- [x] 16. Anti-aliased previews
- [x] 17. Anti-aliased main window
- [ ] 18. Command to export in a video format
- [ ] 19. Insert time delays between keyframes
- [ ] 20. Red line advances while playing

## Point Values
* 1-3: 30pts
* 4-6: 20pts
* 7-10: 20pts
* 11-12: 20pts
* 13: 5pts
* 14: 5pts
* 15: 10pts
* 16: 10pts
* 17: 10pts
* 18: 10pts
* 19: 15pts
* 20: ?

# Building
First run
`mkdir build && cd build`

Then generate the cmake files and `compile_commands.json` for the vscode cpp plugin
`cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=YES ..`

Finally, build
`make -j8`

# Write-up

## Keyframe structure
We created a vector of `Keyframe`s in the `Mesh` struct. Each keyframe contained a vector containing every joint's relative rotation for the frame. It also contained a pointer to a `TextureToRender` object. More on rendering to texture below.

### Saving to / loading from JSON
We simply deconstructed all the quaternions into 4-length arrays and saved the keyframes as JSON object. Loading the file was simply the reverse, and setting the `TextureToRender` pointer to `nullptr` (for reasons we'll discuss below).

## Animation
To animate our model, we kept track of the time of the animation and interpolated the frames corresponding to the two nearest integers. (For example, at time 1.5 seconds we would interpolate keyframes 1 and 2 equally.) We'd then update the skeleton's data with the new relative rotations. `R` to rewind simply paused animation and set time to 0.0 seconds. 

## Preview panel

### Rendering to / from textures
TODO

### Keyframe management
The keyframe management functionality (`F` to add, `Delete` to delete, `U` to update) was simply adding, removing, and modifying the vector of keyframes. `Space` was just updating the skeleton's `T_i` matrices. To detect mouse clicks on certain keyframes, we kept track of how far down we had scrolled, then did some math to figure out the keyframe that was being clicked. This index would be stored, and `PgUp` and `PgDn` decrements/increments this value with bounds checking.

Scrolling over the preview area would change a height value (with bounds checking) that controls what portions of which 3 or 4 keyframes are displayed.

## Extra Credit

### Ctrl+Shift+S to save to custom filename
We used an external library [tinyfiledialogs](http://tinyfiledialogs.sourceforge.net/) to create a pop-up window with prompt. We performed some sanitation, then saved the JSON as usual.

### Cursor
TODO

### Scroll bar
TODO

### Anti-aliased previews
TODO

### Anti-aliased main view
TODO

### Save to video
Install the `ffmpeg` libary with `sudo apt-get install ffmpeg`

We used the `ffmpeg` library to output the animation to an `.mp4` file. We followed (this source)[http://blog.mmacklin.com/2013/06/11/real-time-video-capture-with-ffmpeg/]. We basically run `ffmpeg` and open a pipe from our program as its input. Whenever the user releases `Ctrl+V`, the animation will play from the beginning and frames will be read and sent to `ffmpeg`. When the program is closed, then the pipe is closed and the video is created. 

User input is ignored while recording is taking place until the end is reached.

TODO might need to add something to CMakeLists.txt

### TBD