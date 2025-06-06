import { useState } from 'react';
import { useNavigate } from 'react-router-dom';

export default function Login({ onLogin }: { onLogin: () => void }) {
  const [key, setKey] = useState('');
  const navigate = useNavigate();
  return (
    <div className="flex flex-col items-center justify-center h-screen bg-gray-900 text-white">
      <div className="bg-gray-800 p-6 rounded shadow-md w-80">
        <h1 className="text-2xl mb-4 text-center">ULTSPY Login</h1>
        <input
          type="password"
          className="w-full p-2 mb-4 bg-gray-700 rounded"
          placeholder="API Key"
          value={key}
          onChange={(e) => setKey(e.target.value)}
        />
        <button
          className="w-full bg-blue-600 hover:bg-blue-700 py-2 rounded"
          onClick={() => {
            localStorage.setItem('ULTSPY_API_KEY', key);
            onLogin();
            navigate('/');
          }}
        >
          Login
        </button>
      </div>
    </div>
  );
}
