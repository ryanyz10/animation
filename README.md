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
- [x] 18. Command to export in a video format
- [x] 19. Insert time delays between keyframes
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
When we hit `F`, the current skeleton is saved to a keyframe. Each `KeyFrame` object has an associated `TextureToRender *texture`, which is initialized to `nullptr`. Due to the limited and static size of the preview window, we know we will be drawing at most 4 keyframe previews. We thus employ a kind of lazy-loading to the textures: we figure out which 4 keyframes will be drawn on the screen and check each ones `texture`. If `texture` is `nullptr`, then the texture has never been rendered. We call `TextureToRender::create` to initialize the new texture's framebuffer, depth buffer and texture. Then we draw the floor and mesh to this texture and save it to the keyframe's texture. This method ensures that we don't do too much work in one render loop, while also making sure we don't do any excess work of rendering the same preview repeatedly.

To display the preview, we draw the quad (with vertices set to the correct dimensions in NDC). We apply a shift depending on where the preview is in the list and then texture it with the above generated texture.

We used a pointer to prevent the textures from being de-allocated. Initially, we were copying `TextureToRender` objects into the keyframes. The old `TextureToRender` would then go out of scope and be destructed. This would delete the texture, which was a global name, so attempting to bind would cause GL errors. 

### Keyframe management
The keyframe management functionality (`F` to add, `Delete` to delete, `U` to update) was simply adding, removing, and modifying the vector of keyframes. `Space` was just updating the skeleton's `T_i` matrices. To detect mouse clicks on certain keyframes, we kept track of how far down we had scrolled, then did some math to figure out the keyframe that was being clicked. This index would be stored, and `PgUp` and `PgDn` decrements/increments this value with bounds checking.

Scrolling over the preview area would change a height value (with bounds checking) that controls what portions of which 3 or 4 keyframes are displayed.

## Extra Credit

### Ctrl+Shift+S to save to custom filename
We used an external library [tinyfiledialogs](http://tinyfiledialogs.sourceforge.net/) to create a pop-up window with prompt. We performed some sanitation, then saved the JSON as usual.

### Cursor
A bright green cursor appears when you click in between frames. We inserted a 'buffer' between every keyframe as well as before the first and after the last keyframe. We changed the meaning of `selected_keyframe` to mean the index of the selected object -- buffers had even indices and keyframes had odd indices. We modified our math to handle mouse clicks, and `PgUp` and `PgDn` now alternate between buffer and keyframe. Additionally, we changed the behavior of `F` to insert at the selected point whenever a buffer was selected. 

### Scroll bar
We created a scroll bar on the right side of the preview panel, and its size and position is dependent on how many keyframes there are as well as how far down we've scrolled (both are values we track). We performed some math to transform the quad to the desired position, and the shaders were quite simple.

### Anti-aliased previews
This is an extension of the rendering to/from textures described above. We extended the `TextureToRender::create` method to take a `bool multisample` and `int num_samples`. If `multisample` is set to `true`, we simply call different Gl commands to create a 2D multisample texture instead of a regular multisample texture.

In the render loop, we first create a `TextureToRender multisample_texture` on the stack and render the floor/mesh to it. Then we allocate a new `TextureToRender* texture`, blit the `multisample_texture` onto `texture` and then set the `texture` pointer of the current keyframe. Texturing the preview quads is the same as before

### Anti-aliased main view
This is a similar process to anti-aliased previews. We create a `TextureToRender main_multisample` that multisamples the main scene. We then blit from `main_multisample` onto the default framebuffer to render the scene. We ran into an issue where previous frames remained on-screen -- this was because we weren't clearing both the default framebuffer and `main_multisample` framebuffer. Clearing both fixed this issue for us.

A note about both anti-aliasing, the effect seems to be very slight, at least on my machine where `MAX_SAMPLES` is 8 (not sure why). Maybe with more samples it'll look better, but my guess is that we need to implement a more complex anti-aliasing method as well.

### Save to video
Install the `ffmpeg` libary with `sudo apt-get install ffmpeg`

We used the `ffmpeg` library to output the animation to an `.mp4` file. We followed (this source)[http://blog.mmacklin.com/2013/06/11/real-time-video-capture-with-ffmpeg/]. We basically run `ffmpeg` and open a pipe from our program as its input. Whenever the user releases `Ctrl+V`, the animation will play from the beginning and frames will be read and sent to `ffmpeg`. When the program is closed, then the pipe is closed and the video is created. 

User input is ignored while recording is taking place until the end is reached.

TODO might need to add something to CMakeLists.txt

### Time Delays
To implement delays, we created a vector containing the delays between each frame, as well as a vector tracking the start time of each keyframe. We then updated the `updateAnimation` function to handle these changes -- our original assumption was that each keyframe took one second which no longer held true. To enter a delay, highlight a buffer and press `shift+t`. This will popup with a dialog to enter the delay duration. 

When inserting a keyframe on a delay, the delay is split with half the time coming before and half the time after. When deleting a keyframe, the delays on either side are merged.

We weren't able to get text rendering done (having issues with getting `truetype` to link correctly) so we weren't able to finish that part of it. The idea would've been to render the font to a texture and then texture quads representing the letters.