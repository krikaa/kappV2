import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import WindiCSS from 'vite-plugin-windicss'
import * as path from 'path'
import postcssPresetEnv from 'postcss-preset-env'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [vue(), WindiCSS()],
  resolve: {
    alias: { '@': path.resolve(__dirname, 'src') },
  },
  css: {
    postcss: {
      plugins: [
        postcssPresetEnv // Add postcss-preset-env
      ],
    },
  },
})
