import { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import api from './api';
import Spinner from './components/Spinner';

export default function Login({ onLogin }: { onLogin: () => void }) {
  const [key, setKey] = useState('');
  const [show, setShow] = useState(false);
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);
  const navigate = useNavigate();
  return (
    <div className="flex flex-col items-center justify-center h-screen">
      <div className="bg-[#1a1a1a] p-6 rounded shadow-md w-80">
        <h1 className="text-2xl mb-4 text-center text-neonBlue">ULTSPY Login</h1>
        <div className="relative mb-4">
          <input
            type={show ? 'text' : 'password'}
            className="w-full p-2 bg-[#232323] rounded pr-10"
            placeholder="API Key"
            value={key}
            onChange={(e) => setKey(e.target.value)}
          />
          <button
            type="button"
            onClick={() => setShow(!show)}
            className="absolute right-2 top-2 text-gray-400 hover:text-gray-200"
          >
            {show ? '🙈' : '👁️'}
          </button>
        </div>
        <button
          className="w-full bg-neonBlue hover:bg-cyan-400 py-2 rounded text-[#121212]"
          onClick={async () => {
            setLoading(true);
            setError('');
            try {
              const res = await api.post('/login', { api_key: key });
              localStorage.setItem('ULTSPY_JWT', res.data.token);
              onLogin();
              navigate('/');
            } catch {
              localStorage.removeItem('ULTSPY_JWT');
              setError('Ungültiger API-Key oder keine Verbindung zum Server.');
            } finally {
              setLoading(false);
            }
          }}
        >
          {loading ? <Spinner /> : 'Login'}
        </button>
        {error && <p className="text-red-500 mt-2 text-center text-sm">{error}</p>}
      </div>
    </div>
  );
}
