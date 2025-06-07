import colors from 'tailwindcss/colors'

/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        gray: colors.gray,
        neonBlue: '#00e0ff',
        turquoise: '#1de9b6',
      },
    },
  },
  plugins: [],
}
