import type { ButtonHTMLAttributes } from 'react';

export default function Button({ className = '', ...props }: ButtonHTMLAttributes<HTMLButtonElement>) {
  return (
    <button
      className={`bg-neonBlue text-[#121212] hover:bg-turquoise px-4 py-2 rounded transition-colors ${className}`}
      {...props}
    />
  );
}
