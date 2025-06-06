import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query';
import api from '../api';

export interface ApiKey {
  id: string;
  name: string;
  created_at: string;
  last_used_at: string | null;
  last_ip: string | null;
}

interface KeysResponse {
  keys: ApiKey[];
}

export function useApiKeys() {
  return useQuery({
    queryKey: ['apikeys'],
    queryFn: async () => {
      const { data } = await api.get<KeysResponse>('/admin/apikeys');
      return data.keys;
    },
  });
}

export function useCreateApiKey() {
  const qc = useQueryClient();
  return useMutation({
    mutationFn: async (name: string) => {
      const { data } = await api.post<{ id: string; secret: string }>('/admin/apikeys', { name });
      return data;
    },
    onSuccess: () => {
      qc.invalidateQueries({ queryKey: ['apikeys'] });
    },
  });
}

export function useRevokeApiKey() {
  const qc = useQueryClient();
  return useMutation({
    mutationFn: async (id: string) => {
      await api.delete(`/admin/apikeys/${id}`);
    },
    onSuccess: () => {
      qc.invalidateQueries({ queryKey: ['apikeys'] });
    },
  });
}
