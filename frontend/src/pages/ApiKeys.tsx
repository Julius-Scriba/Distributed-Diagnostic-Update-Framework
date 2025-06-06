import { useState } from 'react';
import { useApiKeys, useCreateApiKey, useRevokeApiKey } from '../api/useApiKeys';
import Spinner from '../components/Spinner';
import { formatDistanceToNow } from 'date-fns';

export default function ApiKeys() {
  const { data, isLoading, error } = useApiKeys();
  const create = useCreateApiKey();
  const revoke = useRevokeApiKey();
  const [name, setName] = useState('');
  const [newSecret, setNewSecret] = useState<string | null>(null);
  const [showStale, setShowStale] = useState(false);

  const keys = data?.slice().sort((a, b) => {
    const aTime = a.last_used_at ? Date.parse(a.last_used_at) : 0;
    const bTime = b.last_used_at ? Date.parse(b.last_used_at) : 0;
    return aTime - bTime;
  });
  const now = Date.now();
  const filtered = showStale
    ? keys?.filter(k => !k.last_used_at || now - Date.parse(k.last_used_at) > 90 * 24 * 3600 * 1000)
    : keys;

  if (isLoading) return <Spinner />;
  if (error) return <div className="text-red-600">Error</div>;

  return (
    <div className="space-y-4">
      <h1 className="text-xl font-bold text-neonBlue">API Keys</h1>
      <form
        onSubmit={e => {
          e.preventDefault();
          create.mutate(name, {
            onSuccess: d => {
              setNewSecret(d.secret);
              setName('');
            },
          });
        }}
        className="space-x-2"
      >
        <input
          value={name}
          onChange={e => setName(e.target.value)}
          placeholder="Name"
          className="bg-[#232323] p-2 rounded"
        />
        <button className="bg-neonBlue text-[#121212] px-3 py-1 rounded" type="submit">
          Create
        </button>
      </form>
      {newSecret && (
        <div className="bg-yellow-100 p-2 border">New key: {newSecret}</div>
      )}
      <label className="block text-sm mb-1">
        <input type="checkbox" checked={showStale} onChange={e => setShowStale(e.target.checked)} className="mr-1" />
        Show stale (&gt;90d unused)
      </label>
      <table className="min-w-full bg-[#232323]">
        <thead>
          <tr>
            <th className="text-left p-2">Name</th>
            <th className="text-left p-2">Created</th>
            <th className="text-left p-2">Last Used</th>
            <th className="text-left p-2">Last IP</th>
            <th></th>
          </tr>
        </thead>
        <tbody>
          {filtered?.map(k => (
            <tr key={k.id} className="border-t">
              <td className="p-2">{k.name}</td>
              <td className="p-2">{formatDistanceToNow(new Date(k.created_at), { addSuffix: true })}</td>
              <td className="p-2">
                {k.last_used_at ? formatDistanceToNow(new Date(k.last_used_at), { addSuffix: true }) : 'never'}
              </td>
              <td className="p-2">{k.last_ip ?? '-'}</td>
              <td className="p-2 text-right">
                <button
                  onClick={() => revoke.mutate(k.id)}
                  className="text-red-500 underline"
                >
                  Revoke
                </button>
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}
