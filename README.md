# TODO
- [x] 1. `F` key saves a keyframe to an ordered list of keyframes
- [x] 2. `P` key plays through the list of keyframes, interpolating between positions
- [x] 3. `R` key rewinds to the beginning of the keyframes
- [ ] 4. render keyframes to a texture
Probably just follow what was said on Piazza (@334, and bug fix in @354)
- [ ] 5. draw the sequence of textures in the right side of the window
Probably some sort of `current_scroll_height` thing that controls how many textures are seen
- [ ] 6. scroll through the keyframe previews with the mouse scroll wheel
Probably just change `current_scroll_height`
- [ ] 7. left-clicking keyframe preview selects the keyframe, page up/page down moves up and down, highlight the selected keyframe
Probably something like `current_bone_`, so `current_frame_`. Similar logic for page up/down, highlight
- [ ] 8. `U` key updates the selected keyframe
Same logic as `F` key, but just overwriting the `current_frame_`
- [ ] 9. `delete` key deletes the selected keyframe
Delete `current_frame_`
- [ ] 10. `space` sets model pose to pose in keyframe
Just force skeleton T_i's to whats in `current_frame_`
- [ ] 11. `ctrl+s` saves keyframe list to `animation.json`
Just write the keyframe T_i's to a json
- [ ] 12. Specifying third cli arg reads in animation from `animation.json`
This will need to create textures for every keyframe, probably part of the loading process
- [ ] 13. Popup dialog for saving
- [ ] 14. Keyframe cursor to allow arbitrary insertion
- [ ] 15. Command to export in a video format
- [ ] 16. Insert time delays between keyframes

## Point Values
* 1-3: 30pts
* 4-6: 20pts
* 7-10: 20pts
* 11-12: 20pts
* 13: 5pts
* 14: 5pts
* 15: 10pts
* 16: 15pts

# Building
First run
`mkdir build && cd build`

Then generate the cmake files and `compile_commands.json` for the vscode cpp plugin
`cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=YES ..`

Finally, build
`make -j8`