// vite.config.ts
import { defineConfig } from "file:///C:/Users/rannr/Documents/tt%C3%BC/sem5/projekt/webUI/node_modules/.pnpm/vite@4.5.0_@types+node@20.8.9/node_modules/vite/dist/node/index.js";
import vue from "file:///C:/Users/rannr/Documents/tt%C3%BC/sem5/projekt/webUI/node_modules/.pnpm/@vitejs+plugin-vue@4.4.0_vite@4.5.0_vue@3.3.7/node_modules/@vitejs/plugin-vue/dist/index.mjs";
import WindiCSS from "file:///C:/Users/rannr/Documents/tt%C3%BC/sem5/projekt/webUI/node_modules/.pnpm/vite-plugin-windicss@1.9.1_vite@4.5.0/node_modules/vite-plugin-windicss/dist/index.mjs";
import * as path from "path";
import postcssPresetEnv from "file:///C:/Users/rannr/Documents/tt%C3%BC/sem5/projekt/webUI/node_modules/.pnpm/postcss-preset-env@9.3.0_postcss@8.4.31/node_modules/postcss-preset-env/dist/index.mjs";
var __vite_injected_original_dirname = "C:\\Users\\rannr\\Documents\\tt\xFC\\sem5\\projekt\\webUI";
var vite_config_default = defineConfig({
  plugins: [vue(), WindiCSS()],
  resolve: {
    alias: { "@": path.resolve(__vite_injected_original_dirname, "src") }
  },
  css: {
    postcss: {
      plugins: [
        postcssPresetEnv
        // Add postcss-preset-env
      ]
    }
  }
});
export {
  vite_config_default as default
};
//# sourceMappingURL=data:application/json;base64,ewogICJ2ZXJzaW9uIjogMywKICAic291cmNlcyI6IFsidml0ZS5jb25maWcudHMiXSwKICAic291cmNlc0NvbnRlbnQiOiBbImNvbnN0IF9fdml0ZV9pbmplY3RlZF9vcmlnaW5hbF9kaXJuYW1lID0gXCJDOlxcXFxVc2Vyc1xcXFxyYW5uclxcXFxEb2N1bWVudHNcXFxcdHRcdTAwRkNcXFxcc2VtNVxcXFxwcm9qZWt0XFxcXHdlYlVJXCI7Y29uc3QgX192aXRlX2luamVjdGVkX29yaWdpbmFsX2ZpbGVuYW1lID0gXCJDOlxcXFxVc2Vyc1xcXFxyYW5uclxcXFxEb2N1bWVudHNcXFxcdHRcdTAwRkNcXFxcc2VtNVxcXFxwcm9qZWt0XFxcXHdlYlVJXFxcXHZpdGUuY29uZmlnLnRzXCI7Y29uc3QgX192aXRlX2luamVjdGVkX29yaWdpbmFsX2ltcG9ydF9tZXRhX3VybCA9IFwiZmlsZTovLy9DOi9Vc2Vycy9yYW5uci9Eb2N1bWVudHMvdHQlQzMlQkMvc2VtNS9wcm9qZWt0L3dlYlVJL3ZpdGUuY29uZmlnLnRzXCI7aW1wb3J0IHsgZGVmaW5lQ29uZmlnIH0gZnJvbSAndml0ZSdcbmltcG9ydCB2dWUgZnJvbSAnQHZpdGVqcy9wbHVnaW4tdnVlJ1xuaW1wb3J0IFdpbmRpQ1NTIGZyb20gJ3ZpdGUtcGx1Z2luLXdpbmRpY3NzJ1xuaW1wb3J0ICogYXMgcGF0aCBmcm9tICdwYXRoJ1xuaW1wb3J0IHBvc3Rjc3NQcmVzZXRFbnYgZnJvbSAncG9zdGNzcy1wcmVzZXQtZW52J1xuXG4vLyBodHRwczovL3ZpdGVqcy5kZXYvY29uZmlnL1xuZXhwb3J0IGRlZmF1bHQgZGVmaW5lQ29uZmlnKHtcbiAgcGx1Z2luczogW3Z1ZSgpLCBXaW5kaUNTUygpXSxcbiAgcmVzb2x2ZToge1xuICAgIGFsaWFzOiB7ICdAJzogcGF0aC5yZXNvbHZlKF9fZGlybmFtZSwgJ3NyYycpIH0sXG4gIH0sXG4gIGNzczoge1xuICAgIHBvc3Rjc3M6IHtcbiAgICAgIHBsdWdpbnM6IFtcbiAgICAgICAgcG9zdGNzc1ByZXNldEVudiAvLyBBZGQgcG9zdGNzcy1wcmVzZXQtZW52XG4gICAgICBdLFxuICAgIH0sXG4gIH0sXG59KVxuIl0sCiAgIm1hcHBpbmdzIjogIjtBQUFvVixTQUFTLG9CQUFvQjtBQUNqWCxPQUFPLFNBQVM7QUFDaEIsT0FBTyxjQUFjO0FBQ3JCLFlBQVksVUFBVTtBQUN0QixPQUFPLHNCQUFzQjtBQUo3QixJQUFNLG1DQUFtQztBQU96QyxJQUFPLHNCQUFRLGFBQWE7QUFBQSxFQUMxQixTQUFTLENBQUMsSUFBSSxHQUFHLFNBQVMsQ0FBQztBQUFBLEVBQzNCLFNBQVM7QUFBQSxJQUNQLE9BQU8sRUFBRSxLQUFVLGFBQVEsa0NBQVcsS0FBSyxFQUFFO0FBQUEsRUFDL0M7QUFBQSxFQUNBLEtBQUs7QUFBQSxJQUNILFNBQVM7QUFBQSxNQUNQLFNBQVM7QUFBQSxRQUNQO0FBQUE7QUFBQSxNQUNGO0FBQUEsSUFDRjtBQUFBLEVBQ0Y7QUFDRixDQUFDOyIsCiAgIm5hbWVzIjogW10KfQo=
