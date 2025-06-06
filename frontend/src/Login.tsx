import { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import api from './api';
import Spinner from './components/Spinner';

export default function Login({ onLogin }: { onLogin: () => void }) {
  const [key, setKey] = useState('');
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);
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
        {error && <p className="text-red-400 mt-2 text-center text-sm">{error}</p>}
      </div>
    </div>
  );
}
