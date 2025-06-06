import { useState } from 'react';
import useAgents from '../api/useAgents';
import useCommands from '../api/useCommands';
import useSendCommand from '../api/useSendCommand';
import Spinner from '../components/Spinner';

const COMMAND_OPTIONS = ['SAFE_MODE', 'DEEP_SLEEP', 'WIPE', 'RECON'];

export default function Commands() {
  const { data: agents } = useAgents();
  const [selected, setSelected] = useState('');
  const {
    data: queue,
    isLoading,
    isError,
    refetch,
  } = useCommands(selected || null);

  const [cmd, setCmd] = useState(COMMAND_OPTIONS[0]);
  const [params, setParams] = useState('{}');
  const [jsonError, setJsonError] = useState('');
  const sendCommand = useSendCommand(selected);

  const submit = () => {
    try {
      const parsed = params ? JSON.parse(params) : {};
      setJsonError('');
      sendCommand.mutate(
        { command: cmd, parameters: parsed },
        { onSuccess: () => refetch() }
      );
    } catch {
      setJsonError('Parameters must be valid JSON');
    }
  };

  return (
    <div className="p-4 text-white space-y-4">
      <h1 className="text-2xl mb-2">Commands</h1>
      <div>
        <label className="mr-2">Select Agent:</label>
        <select
          className="bg-gray-700 p-2 rounded"
          value={selected}
          onChange={(e) => setSelected(e.target.value)}
        >
          <option value="">-- choose --</option>
          {agents?.map((a) => (
            <option key={a.uuid} value={a.uuid}>
              {a.uuid}
            </option>
          ))}
        </select>
      </div>

      {selected && (
        <>
          {isLoading && <Spinner />}
          {isError && <p className="text-red-400">Failed to load commands</p>}
          {queue && (
            <div className="overflow-x-auto mb-4">
            <table className="min-w-full bg-gray-800 text-sm">
              <thead>
                <tr>
                  <th className="py-2">Command</th>
                  <th>Parameters</th>
                  <th>Timestamp</th>
                </tr>
              </thead>
              <tbody>
                {queue.map((c, idx) => (
                  <tr key={idx} className="text-center">
                    <td className="py-1 px-2">{c.command}</td>
                    <td>
                      <pre className="text-left whitespace-pre-wrap">
                        {JSON.stringify(c.parameters || {}, null, 2)}
                      </pre>
                    </td>
                    <td>{c.timestamp || '-'}</td>
                  </tr>
                ))}
              </tbody>
            </table>
            </div>
          )}

          <div className="bg-gray-800 p-4 rounded w-full max-w-md">
            <h2 className="text-xl mb-2">Add Command</h2>
            <select
              className="w-full bg-gray-700 p-2 mb-2 rounded"
              value={cmd}
              onChange={(e) => setCmd(e.target.value)}
            >
              {COMMAND_OPTIONS.map((o) => (
                <option key={o} value={o}>
                  {o}
                </option>
              ))}
            </select>
            <textarea
              className="w-full bg-gray-700 p-2 mb-2 rounded"
              rows={3}
              value={params}
              onChange={(e) => setParams(e.target.value)}
            ></textarea>
            {jsonError && <p className="text-red-400 text-sm">{jsonError}</p>}
            <button
              className="w-full bg-blue-600 hover:bg-blue-700 py-2 rounded mt-2 flex justify-center"
              onClick={submit}
              disabled={sendCommand.isPending}
            >
              {sendCommand.isPending ? (
                <div className="w-4 h-4 border-2 border-white border-t-transparent rounded-full animate-spin"></div>
              ) : (
                'Add'
              )}
            </button>
            {sendCommand.isSuccess && (
              <p className="text-green-400 mt-2 text-center">Command queued</p>
            )}
            {sendCommand.isError && (
              <p className="text-red-400 mt-2 text-center">Error</p>
            )}
          </div>
        </>
      )}
    </div>
  );
}
