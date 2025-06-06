import { useState } from 'react';
import { useTemplates, useCreateTemplate, useDeleteTemplate } from '../api/useTemplates';
import Spinner from '../components/Spinner';

const COMMAND_OPTIONS = ['SAFE_MODE', 'DEEP_SLEEP', 'WIPE', 'RECON'];

export default function Templates() {
  const { data, isLoading, isError } = useTemplates();
  const create = useCreateTemplate();
  const deleter = useDeleteTemplate();

  const [name, setName] = useState('');
  const [cmd, setCmd] = useState(COMMAND_OPTIONS[0]);
  const [params, setParams] = useState('{}');
  const [error, setError] = useState('');

  const submit = () => {
    try {
      const parsed = params ? JSON.parse(params) : {};
      setError('');
      create.mutate({ name, command: cmd, parameters: parsed }, { onSuccess: () => setName('') });
    } catch {
      setError('Parameters must be valid JSON');
    }
  };

  return (
    <div className="p-4 space-y-4">
      <h1 className="text-2xl mb-2 text-neonBlue">Templates</h1>

      <div className="bg-[#1a1a1a] p-4 rounded w-full max-w-md">
        <h2 className="text-xl mb-2">New Template</h2>
        <input
          className="w-full bg-[#232323] p-2 mb-2 rounded"
          placeholder="Name"
          value={name}
          onChange={(e) => setName(e.target.value)}
        />
        <select className="w-full bg-[#232323] p-2 mb-2 rounded" value={cmd} onChange={(e) => setCmd(e.target.value)}>
          {COMMAND_OPTIONS.map(o => <option key={o} value={o}>{o}</option>)}
        </select>
        <textarea
          className="w-full bg-[#232323] p-2 mb-2 rounded"
          rows={3}
          value={params}
          onChange={(e) => setParams(e.target.value)}
        ></textarea>
        {error && <p className="text-red-400 text-sm">{error}</p>}
        <button className="w-full bg-neonBlue hover:bg-cyan-400 py-2 rounded text-[#121212]" onClick={submit} disabled={create.isPending}>
          Add
        </button>
        {create.isSuccess && (
          <p className="text-green-400 text-sm text-center mt-2">Saved</p>
        )}
        {create.isError && <p className="text-red-400 text-sm text-center mt-2">Error</p>}
      </div>

      <div>
        {isLoading && <Spinner />}
        {isError && <p className="text-red-400">Failed to load templates</p>}
        {data && (
          <div className="overflow-x-auto">
          <table className="min-w-full bg-[#232323] text-sm">
            <thead>
              <tr>
                <th className="py-2">Name</th>
                <th>Command</th>
                <th>Parameters</th>
                <th></th>
              </tr>
            </thead>
            <tbody>
              {[...data].reverse().map(t => (
                <tr key={t.template_id} className="text-center">
                  <td className="py-1 px-2">{t.name}</td>
                  <td>{t.command}</td>
                  <td>
                    <pre className="text-left whitespace-pre-wrap">{JSON.stringify(t.parameters, null, 2)}</pre>
                  </td>
                  <td>
                    <button className="text-red-400" onClick={() => deleter.mutate(t.template_id)}>Delete</button>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
          </div>
        )}
      </div>
    </div>
  );
}
