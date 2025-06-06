import { useState } from 'react';
import { useParams } from 'react-router-dom';
import useAgent from '../../api/useAgent';
import useSendCommand from '../../api/useSendCommand';
import Spinner from '../../components/Spinner';
import Modal from '../../components/Modal';
import { formatDistanceToNow } from 'date-fns';

export default function AgentDetail() {
  const { uuid } = useParams<{ uuid: string }>();
  const { data: agent, isLoading, isError } = useAgent(uuid || null);
  const sendCommand = useSendCommand(uuid || '');

  const [modal, setModal] = useState(false);
  const [cmd, setCmd] = useState('SAFE_MODE');
  const [params, setParams] = useState('{}');
  const [jsonError, setJsonError] = useState('');

  if (isLoading) return <Spinner />;
  if (isError) return <p className="text-red-400">Failed to load agent</p>;
  if (!agent) return <p className="text-red-400">Agent not found</p>;

  const stability = () => {
    const seconds = (Date.now() - new Date(agent.last_seen).getTime()) / 1000;
    if (!agent.online || seconds > 180)
      return <span className="bg-red-600 text-xs px-2 py-1 rounded">🔴 Offline</span>;
    if (seconds <= 30)
      return <span className="bg-green-600 text-xs px-2 py-1 rounded">🟢 Stable</span>;
    if (seconds <= 90)
      return <span className="bg-yellow-500 text-xs px-2 py-1 rounded">🟡 Warning</span>;
    return <span className="bg-orange-500 text-xs px-2 py-1 rounded">🟠 Degraded</span>;
  };

  const submit = () => {
    try {
      const parsed = params ? JSON.parse(params) : {};
      setJsonError('');
      sendCommand.mutate(
        { command: cmd, parameters: parsed },
        { onSuccess: () => setModal(false) }
      );
    } catch {
      setJsonError('Parameters must be valid JSON');
    }
  };

  return (
    <div className="p-4 text-white space-y-4">
      <h1 className="text-2xl">Agent {agent.uuid}</h1>

      <div className="bg-gray-800 p-4 rounded w-full max-w-md space-y-1">
        <div className="flex justify-between"><span>UUID</span><span className="font-mono">{agent.uuid}</span></div>
        <div className="flex justify-between"><span>Status</span><span>{agent.online ? 'Online' : 'Offline'} </span></div>
        <div className="flex justify-between"><span>Stability</span><span>{stability()}</span></div>
        <div className="flex justify-between"><span>Last Seen</span><span>{formatDistanceToNow(new Date(agent.last_seen), { addSuffix: true })}</span></div>
        <div className="flex justify-between"><span>Build Version</span><span>n/a</span></div>
        <div className="flex justify-between"><span>Registered At</span><span>n/a</span></div>
      </div>

      <button
        className="bg-blue-600 hover:bg-blue-700 py-2 px-4 rounded"
        onClick={() => setModal(true)}
      >
        Send Command
      </button>

      <Modal open={modal} onClose={() => setModal(false)}>
        <h2 className="text-xl mb-2">Send Command</h2>
        <input
          className="w-full bg-gray-700 p-2 rounded mb-2"
          value={cmd}
          onChange={(e) => setCmd(e.target.value)}
          placeholder="Command"
        />
        <textarea
          className="w-full bg-gray-700 p-2 rounded mb-2"
          rows={3}
          value={params}
          onChange={(e) => setParams(e.target.value)}
        ></textarea>
        {jsonError && <p className="text-red-400 text-sm mb-2">{jsonError}</p>}
        <button
          className="w-full bg-blue-600 hover:bg-blue-700 py-2 rounded"
          onClick={submit}
          disabled={sendCommand.isPending}
        >
          {sendCommand.isPending ? (
            <div className="w-4 h-4 border-2 border-white border-t-transparent rounded-full animate-spin mx-auto"></div>
          ) : (
            'Send'
          )}
        </button>
        {sendCommand.isSuccess && (
          <p className="text-green-400 mt-2 text-center">Command queued</p>
        )}
        {sendCommand.isError && (
          <p className="text-red-400 mt-2 text-center">Error</p>
        )}
      </Modal>
    </div>
  );
}
