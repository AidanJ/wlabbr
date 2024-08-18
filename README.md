# wlabbr

In-place abbreviations completion for wlroots based wayland compositors!


### Configuration File
Used for defining abbreviation keywords for expansion.
Looks for configuration file under `$XDG_CONFIG_HOME/wlabbr.json` if not explicitly specified as a command argument.
#### Example
```json
{
  "keywords": {
    "LGTM": "looks good to me",
    "GTG": "got to go",
    ":)": "🙂"
  }
}
```

### Features
- [ ] Other
    - [x] prevent recursive expansion
    - [ ] graceful program exit (upon kill syscalls)
    - [x] check for default config location in `XDG_CONFIG_HOME`
    - [ ] key case sensitivity
    - [ ] double space for sentence completion?
- [ ] JSON
    - [x] support loading registrations from json files
    - [ ] support templates / complex completions
- [ ] Wayland
    - [ ] displaying possible completions on popup surface
    - [ ] displaying completion confirmation on popup surface
    - [ ] detect support (if focused client reports surrounding text)
    - [ ] content type aware (e.g. no completion while entering passwords)
- [ ] Docs
    - [ ] Specify usage and provide an example configuration file
