# TODO
- [ ] 1. `F` key saves a keyframe to an ordered list of keyframes
- [ ] 2. `P` key plays through the list of keyframes, interpolating between positions
- [ ] 3. `R` key rewinds to the beginning of the keyframes
- [ ] 4. render keyframes to a texture
- [ ] 5. draw the sequence of textures in the right side of the window
- [ ] 6. scroll through the keyframe previews with the mouse scroll wheel
- [ ] 7. left-clicking keyframe preview selects the keyframe, page up/page down moves up and down, highlight the selected keyframe
- [ ] 8. `U` key updates the selected keyframe
- [ ] 9. `delete` key deletes the selected keyframe
- [ ] 10. `space` sets model pose to pose in keyframe
- [ ] 11. `ctrl+s` saves keyframe list to `animation.json`
- [ ] 12. Specifying third cli arg reads in animation from `animation.json`
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