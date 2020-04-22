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