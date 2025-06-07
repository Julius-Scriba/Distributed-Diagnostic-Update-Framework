import type { InputHTMLAttributes } from 'react';

export default function Input({ className = '', ...props }: InputHTMLAttributes<HTMLInputElement>) {
  return (
    <input
      className={`w-full p-2 bg-[#232323] rounded outline-none focus:ring-2 focus:ring-neonBlue ${className}`}
      {...props}
    />
  );
}
