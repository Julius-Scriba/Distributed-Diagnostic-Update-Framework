import { useQuery } from '@tanstack/react-query';
import api from '../api';
import type { Agent } from './useAgents';

interface AgentsResponse {
  agents: Agent[];
}

export default function useAgent(uuid: string | null) {
  return useQuery({
    queryKey: ['agent', uuid],
    queryFn: async () => {
      const { data } = await api.get<AgentsResponse>('/admin/agents');
      const agent = data.agents.find(a => a.uuid === uuid);
      return agent || null;
    },
    enabled: !!uuid,
    refetchInterval: 15000,
    staleTime: 15000,
  });
}
