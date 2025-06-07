import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import { randomBytes } from 'crypto'

const buildHash = randomBytes(4).toString('hex')

export default defineConfig({
  plugins: [react()],
  build: {
    rollupOptions: {
      output: {
        entryFileNames: `assets/[name].[hash].${buildHash}.js`,
        chunkFileNames: `assets/[name].[hash].${buildHash}.js`,
        assetFileNames: `assets/[name].[hash].${buildHash}.[ext]`
      }
    }
  },
  server: {
    proxy: {
      '/api': 'http://localhost:5000'
    }
  }
})
