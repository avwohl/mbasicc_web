# Deployment

Copy the contents of the `web/` directory to any static web server:

```
web/
├── index.html      # Main page
├── style.css       # Styling
├── mbasic-ui.js    # UI controller
├── mbasic.js       # WASM loader (generated)
└── mbasic.wasm     # WebAssembly binary (generated)
```

**Important:** Your web server must serve `.wasm` files with the correct MIME type:
```
Content-Type: application/wasm
```

Most modern web servers handle this automatically. If you encounter issues, configure your server to add this MIME type.
