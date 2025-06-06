/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        neonBlue: '#00e0ff',
        turquoise: '#1de9b6',
      },
    },
  },
  plugins: [],
}
